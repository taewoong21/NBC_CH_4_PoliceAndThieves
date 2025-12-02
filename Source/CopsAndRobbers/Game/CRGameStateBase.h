// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CRGameStateBase.generated.h"

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

#pragma endregion
};
