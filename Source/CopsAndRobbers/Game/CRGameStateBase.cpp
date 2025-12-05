// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CRGameStateBase.h"

#include "Net/UnrealNetwork.h"
#include "CopsAndRobbers.h"

ACRGameStateBase::ACRGameStateBase()
{
	CR_LOG_NET(LogCRNet, Log, TEXT(""));
}

void ACRGameStateBase::HandleBeginPlay()
{
	CR_LOG_NET(LogCRNet, Log, TEXT("Begin"));

	Super::HandleBeginPlay();

	CR_LOG_NET(LogCRNet, Log, TEXT("End"));
}

void ACRGameStateBase::OnRep_ReplicatedHasBegunPlay()
{
	CR_LOG_NET(LogCRNet, Log, TEXT("Begin"));

	Super::OnRep_ReplicatedHasBegunPlay();

	CR_LOG_NET(LogCRNet, Log, TEXT("End"));
}

void ACRGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AlivePlayerControllerCount);
	DOREPLIFETIME(ThisClass, MatchState);
	DOREPLIFETIME(ThisClass, RemainingGameTime);
}

