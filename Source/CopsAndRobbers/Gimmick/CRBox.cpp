// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/CRBox.h"
#include "CopsAndRobbers.h"
#include "Net/UnrealNetwork.h"

ACRBox::ACRBox() : ServerRotationYaw(0.0f), RotationSpeed(30.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetRelativeLocation(FVector(-50.f, -50.f, 50.f));
}

void ACRBox::BeginPlay()
{
	Super::BeginPlay();

	CR_LOG_ROLE(LogCRNet, Log, TEXT(""));
}


void ACRBox::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerRotationYaw);
}


void ACRBox::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() == true)
	{
		AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaSeconds, 0.f));
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	else
	{
		// SetActorRotation(FRotator(0.f, ServerRotationYaw, 0.f));
	}
}

void ACRBox::OnRep_ServerRotationYaw()
{
	CR_LOG_NET(LogCRNet, Log, TEXT("OnRep_ServerRotationYaw(): %f"), ServerRotationYaw);

	SetActorRotation(FRotator(0.f, ServerRotationYaw, 0.f));
}



