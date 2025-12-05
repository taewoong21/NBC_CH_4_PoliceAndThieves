// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CRPlayerState.h"
#include "Net/UnrealNetwork.h"

ACRPlayerState::ACRPlayerState()
{
	bReplicates = true; // PlayerState는 기본적으로 true지만 확인
	TeamRole = ETeamRole::None;
}

void ACRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACRPlayerState, TeamRole);
}

void ACRPlayerState::SetTeamRole(ETeamRole NewRole)
{
	if (HasAuthority())
	{
		TeamRole = NewRole;
		UE_LOG(LogTemp, Warning, TEXT("Player %s assigned role: %d"), *GetPlayerName(), (int32)TeamRole);
	}
}

void ACRPlayerState::OnRep_TeamRole()
{
}
