// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

class UTitleScreenWidget;

UCLASS()
class COPSANDROBBERS_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ATitlePlayerController();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTitleScreenWidget> TitleWidgetClass;

	UPROPERTY()
	UTitleScreenWidget* TitleWidgetInstance;
};
