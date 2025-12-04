// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CRGameModeBase.h"
#include "CRGameModeBase.h"

#include "Game/CRGameStateBase.h"
#include "Player/CRPlayerController.h"
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
			NotificationString = FString::Printf(TEXT("Wait another players for playing."));

			RemainWaitingTimeForPlaying = WaitingTime; // 최소인원이 안된다면 대기 시간 초기화.
		}
		else
		{
			NotificationString = FString::Printf(TEXT("Wait %d seconds for playing."), RemainWaitingTimeForPlaying);

			--RemainWaitingTimeForPlaying;
		}

		if (RemainWaitingTimeForPlaying <= 0)
		{
			NotificationString = FString::Printf(TEXT(""));

			CRGameState->MatchState = EMatchState::Playing;
		}

		NotifyToAllPlayer(NotificationString);

		break;
	}
	case EMatchState::Playing:
	{
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
