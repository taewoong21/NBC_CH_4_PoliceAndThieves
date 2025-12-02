// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CRBox.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class COPSANDROBBERS_API ACRBox : public AActor
{
	GENERATED_BODY()
	
#pragma region ACRBox Overrride
public:	
	ACRBox();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaSeconds) override;

#pragma endregion

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	
	// UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	float RotationSpeed;

	// float NetUpdatePeriod;

	// float AccDeltaSecondSinceReplicated;
	
private:
	UFUNCTION()
	void OnRep_ServerRotationYaw();
};
