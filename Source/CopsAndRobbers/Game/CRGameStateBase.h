// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CRGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	None,
	Waiting,
	Playing,
	Ending,
	End
};
/**
 * 
 */
UCLASS()
class COPSANDROBBERS_API ACRGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
#pragma region AGameStateBase Override

public:
	ACRGameStateBase();

	virtual void HandleBeginPlay() override;

	virtual void OnRep_ReplicatedHasBegunPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion

public:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 AlivePlayerControllerCount = 0;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 RemainingGameTime = 0;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	EMatchState MatchState = EMatchState::Waiting;
};
