// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CRAnimInstance.h"

#include "Player/CRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UCRAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACRCharacter>(GetOwningActor());
	if (IsValid(OwnerCharacter) == true)
	{
		OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UCRAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(OwnerCharacter) == false || IsValid(OwnerCharacterMovementComponent) == false)
	{
		return;
	}

	Velocity = OwnerCharacterMovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	bShouldMove = ((OwnerCharacterMovementComponent->GetCurrentAcceleration().IsNearlyZero()) == false) && (3.f < GroundSpeed);
	bIsFalling = OwnerCharacterMovementComponent->IsFalling();

	AimPitch = OwnerCharacter->GetCurrentAimPitch();
}

void UCRAnimInstance::AnimNotify_CheckMeleeAttackHit()
{
	if (IsValid(OwnerCharacter) == true)
	{
		OwnerCharacter->CheckMeleeAttackHit();
	}
}
