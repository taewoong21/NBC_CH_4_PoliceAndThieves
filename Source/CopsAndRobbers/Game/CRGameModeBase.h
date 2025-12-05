// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CRGameModeBase.generated.h"

class ACRPlayerController;

enum class ETeamRole : uint8;

UCLASS()
class COPSANDROBBERS_API ACRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void BeginPlay() override;

	void OnCharacterDead(ACRPlayerController* InController);

	void EndGameWithWinner(ETeamRole WinnerRole);

private:
	UFUNCTION()
	void OnMainTimerElapsed();

	void NotifyToAllPlayer(const FString& NotificationString);

	void AssignRolesToPlayers();

	void OnTimeLimitExpired();

public:
	FTimerHandle MainTimerHandle;

	FTimerHandle GameEndTimerHandle;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<ACRPlayerController>> AlivePlayerControllers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<ACRPlayerController>> DeadPlayerControllers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WaitingTime = 15;

	int32 RemainWaitingTimeForPlaying = 15;

	int32 MinimumPlayerCountForPlaying = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 EndingTime = 15;

	int32 RemainWaitingTimeForEnding = 15;
};
