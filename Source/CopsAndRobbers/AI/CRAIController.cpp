// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CRAIController.h"

#include "NavigationSystem.h"
#include "Player/CRPlayerState.h"
#include "GameFramework/Character.h"

ACRAIController::ACRAIController()
{
	bWantsPlayerState = true; // CRPlayerState::TeamRole 소유를 위해 PlayerState를 가지도록 함
}

void ACRAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 역할 설정: 시민 (Civilian)
	if (ACRPlayerState* PS = GetPlayerState<ACRPlayerState>())
	{
		PS->SetTeamRole(ETeamRole::Civilian);
	}

	// 1초 뒤 패트롤 시작
	GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ACRAIController::StartPatrol, 1.0f, false);
}

void ACRAIController::OnUnPossess()
{
	Super::OnUnPossess();
	GetWorld()->GetTimerManager().ClearTimer(PatrolTimerHandle);
}

void ACRAIController::StartPatrol()
{
	APawn* PawnCon = GetPawn();
	if (!IsValid(PawnCon)) return;

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	if (NavSystem)
	{
		FNavLocation RandomLoc;
		// 내 위치 기준 반경 2000 내의 랜덤한 이동 가능 위치 탐색
		if (NavSystem->GetRandomPointInNavigableRadius(PawnCon->GetActorLocation(), 2000.0f, RandomLoc))
		{
			MoveToLocation(RandomLoc.Location);
		}
		else
		{
			// 갈 곳을 못 찾으면 잠시 후 재시도
			GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ACRAIController::StartPatrol, 2.0f, false);
		}
	}
}

void ACRAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	// 1~3초 대기 후 다시 이동
	float WaitTime = FMath::RandRange(1.0f, 3.0f);
	GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ACRAIController::StartPatrol, WaitTime, false);
}
