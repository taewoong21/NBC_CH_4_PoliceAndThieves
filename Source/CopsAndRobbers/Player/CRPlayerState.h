// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CRPlayerState.generated.h"


UENUM(BlueprintType)
enum class ETeamRole : uint8
{
    None,
    Civilian,   // AI NPC
    Police,     // Player(Cops)
    Thief       // Player(Robbers)
};

UCLASS()
class COPSANDROBBERS_API ACRPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
    ACRPlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetTeamRole(ETeamRole NewRole);
    ETeamRole GetTeamRole() const { return TeamRole; }

    UFUNCTION()
    void OnRep_TeamRole();

protected:
    UPROPERTY(ReplicatedUsing = OnRep_TeamRole, VisibleAnywhere, BlueprintReadOnly, Category = "Role")
    ETeamRole TeamRole;
};
