// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CRStatusComponent.h"
#include "Net/UnrealNetwork.h"
#include "CopsAndRobbers.h"

UCRStatusComponent::UCRStatusComponent() : CurrentHP(100.f), MaxHP(100.f)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

float UCRStatusComponent::ApplyDamage(float InDamage)
{
	if(IsValid(GetOwner()) == false || GetOwner()->HasAuthority() == false)
	{
		return 0.f;
	}

	const float PreviousHP = CurrentHP;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, PreviousHP);

	SetCurrentHP(PreviousHP - ActualDamage);

	return ActualDamage;
}

void UCRStatusComponent::SetCurrentHP(float InCurrentHP)
{
	if (IsValid(GetOwner()) == false || GetOwner()->HasAuthority() == false)
	{
		return;
	}

	CurrentHP = InCurrentHP;
	if (CurrentHP <= KINDA_SMALL_NUMBER)
	{
		CurrentHP = 0.f;
		OnOutOfCurrentHP.Broadcast();
	}
	OnCurrentHPChanged.Broadcast(CurrentHP);
}

void UCRStatusComponent::SetMaxHP(float InMaxHP)
{
	MaxHP = InMaxHP;

	if (MaxHP < KINDA_SMALL_NUMBER)
	{
		MaxHP = 0.1f;
	}

	OnMaxHPChanged.Broadcast(MaxHP);
}

void UCRStatusComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentHP);
	DOREPLIFETIME(ThisClass, MaxHP);
}

void UCRStatusComponent::OnRep_CurrentHP()
{
	CR_LOG_SUBOBJECT_ROLE(LogCRNet, Log, TEXT("CurrentHP: %.1f"), CurrentHP);
	OnCurrentHPChanged.Broadcast(CurrentHP);
}

void UCRStatusComponent::OnRep_MaxHP()
{
	CR_LOG_SUBOBJECT_ROLE(LogCRNet, Log, TEXT("MaxHP: %.1f"), MaxHP);
	OnMaxHPChanged.Broadcast(MaxHP);
}



