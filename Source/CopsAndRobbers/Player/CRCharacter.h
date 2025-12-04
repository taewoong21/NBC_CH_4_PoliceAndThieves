// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CRCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UCRStatusComponent;
class UCRHPTextWidgetComponent;
class UUW_HPText;

UCLASS()
class COPSANDROBBERS_API ACRCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region ACharacter Override
public:
	ACRCharacter();

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

#pragma endregion

#pragma region PlayerCharacter Components
public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }

	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCRStatusComponent> StatusComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCRHPTextWidgetComponent> HPTextWidgetComponent;

#pragma endregion

#pragma region HPWidget

public:
	void SetHPTextWidget(UUW_HPText* InHPTextWidget);

	void TakeBuff(float InBuffValue);

#pragma endregion

#pragma region Input
public:
	float GetCurrentAimPitch() const { return CurrentAimPitch; }

private:
	void HandleMoveInput(const FInputActionValue& InValue);

	void HandleLookInput(const FInputActionValue& InValue);

	void HandleLandMineInput(const FInputActionValue& InValue);

	void HandleMeleeAttackInput(const FInputActionValue& InValue);

	UFUNCTION(Server, Unreliable)
	void ServerRPCUpdateAimValue(const float& InAimPitchValue);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> LandMineAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Input")
	TObjectPtr<UInputAction> MeleeAttackAction;

	UPROPERTY(Replicated)
	float CurrentAimPitch = 0.f;

	float PreviousAimPitch = 0.f;

#pragma endregion

#pragma region LandMine
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCSpawnLandMine();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> LandMineClass;

#pragma endregion

#pragma region Attack

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void CheckMeleeAttackHit();

private:
	void DrawDebugMeleeAttack(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCMeleeAttack(float InStartMeleeAttackTime);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCMeleeAttack();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCPerformMeleeHit(ACharacter* InDamagedCharacters, float InCheckTime);

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayMeleeAttackMontage(ACRCharacter* InTargetCharacter);

	UFUNCTION()
	void OnRep_CanAttack();

	void PlayMeleeAttackMontage();

	UFUNCTION()
	void OnDeath();

protected:
	//bool bCanAttack;
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MeleeAttackMontage;

	float MeleeAttackMontagePlayTime;

	float LastStartMeleeAttackTime;

	float MeleeAttackTimeDifference;

	float MinAllowedTimeForMeleeAttack;

#pragma endregion
};
