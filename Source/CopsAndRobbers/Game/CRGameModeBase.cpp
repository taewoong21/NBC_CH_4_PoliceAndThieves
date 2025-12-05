// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CRGameModeBase.h"
#include "CRGameModeBase.h"

#include "Game/CRGameModeBase.h"
#include "Game/CRGameStateBase.h"
#include "Game/CRGameStateBase.h"
#include "Player/CRPlayerController.h"
#include "Player/CRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "CopsAndRobbers.h"

void ACRGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ACRGameStateBase* CRGameState = GetGameState<ACRGameStateBase>();
	if (IsValid(CRGameState) == false)
	{
		return;
	}

	if (CRGameState->MatchState != EMatchState::Waiting)
	{
		NewPlayer->SetLifeSpan(0.1f);
		return;
	}

	ACRPlayerController* NewPlayerController = Cast<ACRPlayerController>(NewPlayer);
	if (IsValid(NewPlayerController) == true)
	{
		AlivePlayerControllers.Add(NewPlayerController);

		NewPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
	}
}

void ACRGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ACRPlayerController* ExitingPlayerController = Cast<ACRPlayerController>(Exiting);
	if (IsValid(ExitingPlayerController) == true && AlivePlayerControllers.Find(ExitingPlayerController) != INDEX_NONE)
	{
		AlivePlayerControllers.Remove(ExitingPlayerController);
		DeadPlayerControllers.Add(ExitingPlayerController);
	}
}

void ACRGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(MainTimerHandle, this, &ThisClass::OnMainTimerElapsed, 1.f, true);

	RemainWaitingTimeForPlaying = WaitingTime;

	RemainWaitingTimeForEnding = EndingTime;
}

void ACRGameModeBase::OnCharacterDead(ACRPlayerController* InController)
{
	if (IsValid(InController) == false || AlivePlayerControllers.Find(InController) == INDEX_NONE)
	{
		return;
	}
	InController->ClientRPCShowGameResultWidget(AlivePlayerControllers.Num());

	AlivePlayerControllers.Remove(InController);
	DeadPlayerControllers.Add(InController);
}

void ACRGameModeBase::EndGameWithWinner(ETeamRole WinnerRole)
{
	ACRGameStateBase* CRGameState = GetGameState<ACRGameStateBase>();
	if (CRGameState && CRGameState->MatchState == EMatchState::Playing)
	{
		CRGameState->MatchState = EMatchState::Ending;

		// 결과 통보
		FString ResultStr = (WinnerRole == ETeamRole::Police) ? TEXT("Police(Cops) Wins!") : TEXT("Thieves(Robbers) Win!");
		NotifyToAllPlayer(ResultStr);

		
		for (auto PC : AlivePlayerControllers)
		{
			ACRPlayerState* PS = PC->GetPlayerState<ACRPlayerState>();
			// 승리 조건: (경찰승리 + Role == 경찰) 혹은 (도둑승리 & Role == 도둑)
			bool bIsWinner = (PS && PS->GetTeamRole() == WinnerRole);
			PC->ClientRPCShowGameResultWidget(bIsWinner ? 1 : 2); // 1: 승리, 2: 패배 
		}
	}
}

void ACRGameModeBase::OnMainTimerElapsed()
{
	ACRGameStateBase* CRGameState = GetGameState<ACRGameStateBase>();
	if (IsValid(CRGameState) == false)
	{
		return;
	}

	switch (CRGameState->MatchState)
	{
	case EMatchState::None:
		break;
	case EMatchState::Waiting:
	{
		FString NotificationString = FString::Printf(TEXT(""));

		if (AlivePlayerControllers.Num() < MinimumPlayerCountForPlaying)
		{
			NotificationString = FString::Printf(TEXT("Wait another players for playing. (Min 2)"));

			RemainWaitingTimeForPlaying = WaitingTime; // 최소인원 미달 시, 대기 시간 초기화.
		}
		else
		{
			NotificationString = FString::Printf(TEXT("Game starts in %d..."), RemainWaitingTimeForPlaying);

			--RemainWaitingTimeForPlaying;
		}

		if (RemainWaitingTimeForPlaying <= 0)
		{
			// 게임 시작 시, 역할 분배
			AssignRolesToPlayers();

			CRGameState->MatchState = EMatchState::Playing;

			// 제한 시간 타이머 (120초 후 도둑 승리)
			CRGameState->RemainingGameTime = 120;
			NotifyToAllPlayer(TEXT("Find the THIEF! / Hide from POLICE!"));
		}
		else
		{
			NotifyToAllPlayer(NotificationString);
		}

		break;
	}
	case EMatchState::Playing:
	{
		// 도주 타미어 시간 차감
		if (CRGameState->RemainingGameTime > 0)
		{
			CRGameState->RemainingGameTime--;
		}
		else
		{
			// 시간 종료 시, 도둑 승리
			OnTimeLimitExpired();
			return;
		}

		// 생존자 수 표시
		CRGameState->AlivePlayerControllerCount = AlivePlayerControllers.Num();
		FString NotificationString = FString::Printf(TEXT("%d / %d"), CRGameState->AlivePlayerControllerCount, CRGameState->AlivePlayerControllerCount + DeadPlayerControllers.Num());
		NotifyToAllPlayer(NotificationString);

		if (CRGameState->AlivePlayerControllerCount <= 1)
		{
			CRGameState->MatchState = EMatchState::Ending;
		}

		if (CRGameState->AlivePlayerControllerCount <= 1)
		{
			CRGameState->MatchState = EMatchState::Ending;

			AlivePlayerControllers[0]->ClientRPCShowGameResultWidget(1);
		}

		break;
	}
	case EMatchState::Ending:
	{
		FString NotificationString = FString::Printf(TEXT("Waiting %d for returning to title."), RemainWaitingTimeForEnding);

		NotifyToAllPlayer(NotificationString);

		--RemainWaitingTimeForEnding;

		if (RemainWaitingTimeForEnding <= 0)
		{
			for (auto AliveController : AlivePlayerControllers)
			{
				AliveController->ClientRPCReturnToTitle();
			}
			for (auto DeadController : DeadPlayerControllers)
			{
				DeadController->ClientRPCReturnToTitle();
			}

			MainTimerHandle.Invalidate();
			
			FName CurrentLevelName = FName(UGameplayStatics::GetCurrentLevelName(this));
			UGameplayStatics::OpenLevel(this, CurrentLevelName, true, FString(TEXT("listen")));

			return;
		}

		break;
	}
	case EMatchState::End:
		break;
	default:
		break;
	}
}

void ACRGameModeBase::NotifyToAllPlayer(const FString& NotificationString)
{
	for (auto AlivePlayerController : AlivePlayerControllers)
	{
		AlivePlayerController->NotificationText = FText::FromString(NotificationString);
	}

	for (auto DeadPlayerController : DeadPlayerControllers)
	{
		DeadPlayerController->NotificationText = FText::FromString(NotificationString);
	}
}

void ACRGameModeBase::AssignRolesToPlayers()
{
	// 플레이어 목록 셔플
	int32 NumPlayers = AlivePlayerControllers.Num();
	for (int32 i = 0; i < NumPlayers; ++i)
	{
		int32 J = FMath::RandRange(0, NumPlayers - 1);
		AlivePlayerControllers.Swap(i, J);
	}

	// 0번은 경찰, 나머지(1번)는 도둑 
	for (int32 i = 0; i < NumPlayers; ++i)
	{
		ACRPlayerState* PS = AlivePlayerControllers[i]->GetPlayerState<ACRPlayerState>();
		if (PS)
		{
			if (i == 0) // 경찰 선정
			{
				PS->SetTeamRole(ETeamRole::Police);
				AlivePlayerControllers[i]->RoleText = FText::FromString(TEXT("POLICE(Cops)"));
				AlivePlayerControllers[i]->NotificationText = FText::FromString(TEXT("YOU ARE POLICE"));
			}
			else // 도둑 선정
			{
				PS->SetTeamRole(ETeamRole::Thief);
				AlivePlayerControllers[i]->RoleText = FText::FromString(TEXT("THIEF(Robbers)"));
				AlivePlayerControllers[i]->NotificationText = FText::FromString(TEXT("YOU ARE THIEF"));
			}
		}
	}
}

void ACRGameModeBase::OnTimeLimitExpired()
{
	// 제한 시간 종료 시 도둑 승리
	EndGameWithWinner(ETeamRole::Thief);
}
