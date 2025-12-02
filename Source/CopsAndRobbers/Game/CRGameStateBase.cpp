// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CRGameStateBase.h"
#include "CopsAndRobbers.h"

ACRGameStateBase::ACRGameStateBase()
{
	CR_LOG_NET(LogCRNet, Log, TEXT(""));
}

void ACRGameStateBase::HandleBeginPlay()
{
	CR_LOG_NET(LogCRNet, Log, TEXT("Begin"));

	Super::HandleBeginPlay();
	// 서버 로직. 여기서 월드의 모든 액터들에게 BeginPlay() 함수 호출 지시.
	// 이를 통해 ADXGameStateBase::OnRep_ReplicatedHasBegunPlay() 함수가 호출됨.

	CR_LOG_NET(LogCRNet, Log, TEXT("End"));
}

void ACRGameStateBase::OnRep_ReplicatedHasBegunPlay()
{
	CR_LOG_NET(LogCRNet, Log, TEXT("Begin"));

	Super::OnRep_ReplicatedHasBegunPlay();

	CR_LOG_NET(LogCRNet, Log, TEXT("End"));
}
