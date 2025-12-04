// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/CRLandMine.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"

ACRLandMine::ACRLandMine()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(GetRootComponent());

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BoxCollision);

	Particle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	Particle->SetupAttachment(GetRootComponent());
	Particle->SetAutoActivate(false);
}

void ACRLandMine::BeginPlay()
{
	Super::BeginPlay();

	// UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ADXLandMine::BeginPlay()")), true, true, FLinearColor::Green, 5.f);

	if (HasAuthority() == true)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on server.")), true, true, FLinearColor::Green, 5.f);
	}
	else
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (IsValid(OwnerPawn) == true)
		{
			if (OwnerPawn->IsLocallyControlled() == true)
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on owning client.")), true, true, FLinearColor::Green, 5.f);
			}
			else
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on other client.")), true, true, FLinearColor::Green, 5.f);
			}
		}
	}

	if (false == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandMineBeginOverlap))
	{
		OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnLandMineBeginOverlap);
	}
}

void ACRLandMine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ADXLandMine::EndPlay()")), true, true, FLinearColor::Green, 5.f);

	if (true == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandMineBeginOverlap))
	{
		OnActorBeginOverlap.RemoveDynamic(this, &ThisClass::OnLandMineBeginOverlap);
	}
}

void ACRLandMine::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsExploded);
}

void ACRLandMine::OnLandMineBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (HasAuthority() == true)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on server.")), true, true, FLinearColor::Green, 5.f);

		MulticastRPCSpawnEffect();

		if (bIsExploded == false)
		{
			bIsExploded = true;
		}
	}
	else
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (IsValid(OwnerPawn) == true)
		{
			if (OwnerPawn->IsLocallyControlled() == true)
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on owning client.")), true, true, FLinearColor::Green, 5.f);
			}
			else
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on other client.")), true, true, FLinearColor::Green, 5.f);
			}
		}

		if (bIsExploded == false)
		{
			Particle->Activate(true);
		}
	}
}

void ACRLandMine::OnRep_IsExploded()
{
	if (true == bIsExploded && IsValid(ExplodedMaterial) == true)
	{
		Mesh->SetMaterial(0, ExplodedMaterial);
	}
}

void ACRLandMine::MulticastRPCSpawnEffect_Implementation()
{

}
