// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CRAIController.generated.h"

/**
 * 
 */
UCLASS()
class COPSANDROBBERS_API ACRAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ACRAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void StartPatrol();

	// 이동 완료 시 호출 델리게이트 바인딩 함수
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	FTimerHandle PatrolTimerHandle;
};
