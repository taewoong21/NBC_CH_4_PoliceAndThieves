// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/CRBox.h"
#include "CopsAndRobbers.h"
#include "Net/UnrealNetwork.h"

ACRBox::ACRBox() : ServerRotationYaw(0.0f), RotationSpeed(30.0f), AccDeltaSecondSinceReplicated(0.0f), NetCullDistance(1000.f)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetRelativeLocation(FVector(-50.f, -50.f, 50.f));

	const static float BoxActorNetUpdateFrequency = 1.f;
	SetNetUpdateFrequency(BoxActorNetUpdateFrequency); // 1초에 1번씩 액터 레플리케이션 시도. 즉, 서버 성능이 아무리 좋아도 1초에 1번씩만 레플리케이션함.
	NetUpdatePeriod = 1 / GetNetUpdateFrequency(); // 주기 = 1 / 주파수

	SetNetCullDistanceSquared(NetCullDistance * NetCullDistance);
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
		if (NetUpdatePeriod < KINDA_SMALL_NUMBER)
		{
			return;
		}

		AccDeltaSecondSinceReplicated += DeltaSeconds;
		const float LerpRatio = FMath::Clamp(AccDeltaSecondSinceReplicated / NetUpdatePeriod, 0.f, 1.f);

		const float NextServerRotationYaw = ServerRotationYaw + RotationSpeed * NetUpdatePeriod;

		const float EstimatedClientRotationYaw = FMath::Lerp(ServerRotationYaw, NextServerRotationYaw, LerpRatio);
		SetActorRotation(FRotator(0.f, EstimatedClientRotationYaw, 0.f));
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), NetCullDistance / 2.f, 16, FColor::Green, false, -1.f); // NetCullDistanceSquared를 시각화 하기 위한 디버그 드로잉
}

bool ACRBox::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	bool bIsNetRelevant = Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	if (false == bIsNetRelevant)
	{
		CR_LOG_NET(LogCRNet, Log, TEXT("%s is not relevant for(%s, %s)"), *GetName(), *RealViewer->GetName(), *ViewTarget->GetName());
	}

	return bIsNetRelevant;
}

void ACRBox::OnRep_ServerRotationYaw()
{
	CR_LOG_NET(LogCRNet, Log, TEXT("OnRep_ServerRotationYaw(): %f"), ServerRotationYaw);

	SetActorRotation(FRotator(0.f, ServerRotationYaw, 0.f));

	AccDeltaSecondSinceReplicated = 0.f;
}



