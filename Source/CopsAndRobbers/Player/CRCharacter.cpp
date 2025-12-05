// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CRCharacter.h"
#include "CRCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/CRPlayerController.h"
#include "Player/CRPlayerState.h" 
#include "Game/CRGameModeBase.h"
#include "Game/CRGameStateBase.h"
#include "Gimmick/CRLandMine.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Component/CRStatusComponent.h"
#include "Component/CRHPTextWidgetComponent.h"
#include "UI/UW_HPText.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "CopsAndRobbers.h"

ACRCharacter::ACRCharacter() : bCanAttack(true), MeleeAttackMontagePlayTime(0.f), LastStartMeleeAttackTime(0.f), MeleeAttackTimeDifference(0.f), MinAllowedTimeForMeleeAttack(0.02f)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->SetIsReplicated(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetupAttachment(GetRootComponent());
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = false;
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	StatusComponent = CreateDefaultSubobject<UCRStatusComponent>(TEXT("StatusComponent"));

	HPTextWidgetComponent = CreateDefaultSubobject<UCRHPTextWidgetComponent>(TEXT("HPTextWidgetComponent"));
	HPTextWidgetComponent->SetupAttachment(GetRootComponent());
	HPTextWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	// HPTextWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		// Billboard 방식으로 보이나, 주인공 캐릭터를 가리게됨. 또한 UI와 멀어져도 동일한 크기가 유지되는 문제도 있음.
	HPTextWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPTextWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StatusComponent->OnOutOfCurrentHP.AddUObject(this, &ThisClass::OnDeath);
}

void ACRCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocallyControlled() == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());

		// PC가 유효한 경우(Player Character)
		if (IsValid(PC))
		{
			UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

			if (IsValid(EILPS))
			{
				EILPS->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (IsValid(MeleeAttackMontage) == true)
	{
		MeleeAttackMontagePlayTime = MeleeAttackMontage->GetPlayLength();
	}
}

void ACRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);

	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);

	EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	EIC->BindAction(LandMineAction, ETriggerEvent::Started, this, &ThisClass::HandleLandMineInput);

	EIC->BindAction(MeleeAttackAction, ETriggerEvent::Started, this, &ThisClass::HandleMeleeAttackInput);
}

void ACRCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentAimPitch);
	DOREPLIFETIME(ThisClass, bCanAttack);
}

void ACRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(GetController()) == true)
	{
		PreviousAimPitch = CurrentAimPitch;

		FRotator ControlRotation = GetController()->GetControlRotation();
		float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
		CurrentAimPitch = FMath::Clamp(NormalizedPitch, -90.0f, 90.0f);
	}

	if (IsLocallyControlled() == true && PreviousAimPitch != CurrentAimPitch)
	{
		ServerRPCUpdateAimValue(CurrentAimPitch);
	}

	if (IsValid(HPTextWidgetComponent) == true && HasAuthority() == false)
	{
		FVector WidgetComponentLocation = HPTextWidgetComponent->GetComponentLocation();
		FVector LocalPlayerCameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
		HPTextWidgetComponent->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(WidgetComponentLocation, LocalPlayerCameraLocation));
	}
}

void ACRCharacter::SetHPTextWidget(UUW_HPText* InHPTextWidget)
{
	UUW_HPText* HPWidget = Cast<UUW_HPText>(InHPTextWidget);
	if (IsValid(HPWidget) == true)
	{
		HPWidget->InitializeHPTextWidget(StatusComponent);
		StatusComponent->OnCurrentHPChanged.AddUObject(HPWidget, &UUW_HPText::OnCurrentHPChange);
		StatusComponent->OnMaxHPChanged.AddUObject(HPWidget, &UUW_HPText::OnMaxHPChange);
	}
}

void ACRCharacter::TakeBuff(float InBuffValue)
{
	if (IsValid(StatusComponent) == true)
	{
		StatusComponent->SetMaxHP(StatusComponent->GetMaxHP() + InBuffValue);
		StatusComponent->SetCurrentHP(StatusComponent->GetCurrentHP() + InBuffValue);
	}
}

void ACRCharacter::HandleMoveInput(const FInputActionValue& InValue)
{
	if (IsValid(Controller) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}

	const FVector2D InMovementVector = InValue.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InMovementVector.X);
	AddMovementInput(RightDirection, InMovementVector.Y);
}

void ACRCharacter::HandleLookInput(const FInputActionValue& InValue)
{
	if (IsValid(Controller) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}
	
	const FVector2D InLookVector = InValue.Get<FVector2D>();

	AddControllerYawInput(InLookVector.X);
	AddControllerPitchInput(InLookVector.Y);
}

void ACRCharacter::HandleLandMineInput(const FInputActionValue& InValue)
{
	if (IsLocallyControlled() == true)
	{
		ServerRPCSpawnLandMine();
	}
}

void ACRCharacter::HandleMeleeAttackInput(const FInputActionValue& InValue)
{
	if (true == bCanAttack && GetCharacterMovement()->IsFalling() == false)
	{
		ServerRPCMeleeAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());

		if (HasAuthority() == false && IsLocallyControlled() == true)
		{
			PlayMeleeAttackMontage();
		}
	}
}

float ACRCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("TakeDamage: %f"), DamageAmount), true, true, FLinearColor::Red, 5.f);

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	ACRGameStateBase* CRGameState = Cast<ACRGameStateBase>(UGameplayStatics::GetGameState(this));
	if (IsValid(CRGameState) == true && CRGameState->MatchState == EMatchState::Playing)
	{
		StatusComponent->ApplyDamage(ActualDamage);
	}

	return ActualDamage;
}

void ACRCharacter::CheckMeleeAttackHit()
{
	if (IsLocallyControlled() == true)
	{
		TArray<FHitResult> OutHitResults;
		TSet<ACharacter*> DamagedCharacters;
		FCollisionQueryParams Params(NAME_None, false, this);

		const float MeleeAttackRange = 50.f;
		const float MeleeAttackRadius = 50.f;
		// const float MeleeAttackDamage = 10.f;
		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * MeleeAttackRange;

		bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(MeleeAttackRadius), Params);
		if (bIsHitDetected == true)
		{
			for (auto const& OutHitResult : OutHitResults)
			{
				ACharacter* DamagedCharacter = Cast<ACharacter>(OutHitResult.GetActor());
				if (IsValid(DamagedCharacter) == true)
				{
					DamagedCharacters.Add(DamagedCharacter);
				}
			}

			FDamageEvent DamageEvent;
			for (auto const& DamagedCharacter : DamagedCharacters)
			{
				//DamagedCharacter->TakeDamage(MeleeAttackDamage, DamageEvent, GetController(), this);
				ServerRPCPerformMeleeHit(DamagedCharacter, GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
			}
		}

		FColor DrawColor = bIsHitDetected ? FColor::Green : FColor::Red;
		DrawDebugMeleeAttack(DrawColor, Start, End, Forward);
	}
}

void ACRCharacter::DrawDebugMeleeAttack(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward)
{
	const float MeleeAttackRange = 50.f;
	const float MeleeAttackRadius = 50.f;
	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = MeleeAttackRange * 0.5f;
	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, MeleeAttackRadius, FRotationMatrix::MakeFromZ(Forward).ToQuat(), DrawColor, false, 5.0f);
}

bool ACRCharacter::ServerRPCMeleeAttack_Validate(float InStartMeleeAttackTime)
{
	if (LastStartMeleeAttackTime == 0.0f)
	{
		// 최초 공격은 일단 통과.
		return true;
	}

	// 공격재개가능시간 = 몽타주재생시간 - 서버딜레이
	return (MeleeAttackMontagePlayTime - 0.1f) < (InStartMeleeAttackTime - LastStartMeleeAttackTime);
}

void ACRCharacter::ServerRPCPerformMeleeHit_Implementation(ACharacter* InDamagedCharacters, float InCheckTime)
{
	if (!IsValid(InDamagedCharacters)) return;
	
	ACRGameModeBase* GM = Cast<ACRGameModeBase>(GetWorld()->GetAuthGameMode());
	ACRPlayerState* PS = GetPlayerState<ACRPlayerState>();
	ACRPlayerState* TargetPS = InDamagedCharacters->GetPlayerState<ACRPlayerState>();

	// 대기(Waiting)중이면 무시
	ACRGameStateBase* GS = GetWorld()->GetGameState<ACRGameStateBase>();
	if (!GS || GS->MatchState != EMatchState::Playing) return;

	if (GM && PS && TargetPS)
	{
		// 공격자가 경찰인 경우
		if (PS->GetTeamRole() == ETeamRole::Police)
		{
			if (TargetPS->GetTeamRole() == ETeamRole::Thief)
			{
				// 도둑 검거 -> 경찰 승리
				GM->EndGameWithWinner(ETeamRole::Police);

				MulticastRPCMeleeAttack();
			}
			else if (TargetPS->GetTeamRole() == ETeamRole::Civilian)
			{
				// 시민 공격 시, 패널티 - 경찰 본인 데미지 
				FDamageEvent DamageEvent;
				this->TakeDamage(40.0f, DamageEvent, GetController(), this);

				InDamagedCharacters->Destroy();
			}
		}
		// 도둑끼리 혹은 시민이 때리는 경우는 데미지 X
	}
}

bool ACRCharacter::ServerRPCPerformMeleeHit_Validate(ACharacter* InDamagedCharacters, float InCheckTime)
{
	return MinAllowedTimeForMeleeAttack < (InCheckTime - LastStartMeleeAttackTime);
}

void ACRCharacter::ClientRPCPlayMeleeAttackMontage_Implementation(ACRCharacter* InTargetCharacter)
{
	if (IsValid(InTargetCharacter) == true)
	{
		InTargetCharacter->PlayMeleeAttackMontage();
	}
}

void ACRCharacter::OnRep_CanAttack()
{
	if (bCanAttack == true)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void ACRCharacter::PlayMeleeAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(MeleeAttackMontage);
	}
}

void ACRCharacter::OnDeath()
{
	ACRPlayerController* PlayerController = GetController<ACRPlayerController>();
	if (IsValid(PlayerController) == true && HasAuthority() == true)
	{
		PlayerController->OnCharacterDead();
	}
}

void ACRCharacter::MulticastRPCMeleeAttack_Implementation()
{
	if (HasAuthority() == false && IsLocallyControlled() == false)
	{
		PlayMeleeAttackMontage();
	}
}

void ACRCharacter::ServerRPCMeleeAttack_Implementation(float InStartMeleeAttackTime)
{
	// 서버딜레이 = 현재서버시간 - 공격입력이들어왔을때서버시간
	MeleeAttackTimeDifference = GetWorld()->GetTimeSeconds() - InStartMeleeAttackTime;
	MeleeAttackTimeDifference = FMath::Clamp(MeleeAttackTimeDifference, 0.f, MeleeAttackMontagePlayTime);

	if (KINDA_SMALL_NUMBER < MeleeAttackMontagePlayTime - MeleeAttackTimeDifference)
	{
		// Calling SetTimer with a rate less than or equal to zero is identical to calling ClearTimer.
		// 대충 0초로 타이머를 걸면, ClearTimer() 함수를 호출하는 것과 똑같다는 뜻.

		bCanAttack = false;
		OnRep_CanAttack();

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				bCanAttack = true;
				OnRep_CanAttack();
			}), MeleeAttackMontagePlayTime - MeleeAttackTimeDifference, false, -1.f);
	}
	// else
	// {
	//    이 경우는 서버딜레이가 너무 심해버려서, 서버딜레이 == 밀리공격애님몽타주재생시간인 경우.
	//    우리가 할 수 있는건 bCanAttack을 그대로 둬서, 마치 아무일도 일어나지 않게끔함. 바로 공격할 수 있게함.
	// }

	LastStartMeleeAttackTime = InStartMeleeAttackTime;
	PlayMeleeAttackMontage();

	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (IsValid(PlayerController) == true && GetController() != PlayerController)  // 이 캐릭터는 공격한 플레이어의 캐릭터임. 공격한 플레이어의 컨트롤러 외의 컨트롤러들을 찾기 위한 조건문.
		{
			ACRCharacter* OtherPlayerCharacter = Cast<ACRCharacter>(PlayerController->GetPawn());
			if (OtherPlayerCharacter)
			{
				OtherPlayerCharacter->ClientRPCPlayMeleeAttackMontage(this); // 다른 플레이어 컨트롤러의 캐릭터에 공격한 클라이언트의 캐릭터를 넘겨줘서, 애니메이션이 재생되게끔 함.
			}
		}
	}
}

void ACRCharacter::ServerRPCUpdateAimValue_Implementation(const float& InAimPitchValue)
{
	CurrentAimPitch = InAimPitchValue;
}

bool ACRCharacter::ServerRPCSpawnLandMine_Validate()
{
	return true;
}

void ACRCharacter::ServerRPCSpawnLandMine_Implementation()
{
	if (IsValid(LandMineClass) == true)
	{
		FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 300.f) - FVector(0.f, 0.f, 90.f);
		ACRLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ACRLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);

		SpawnedLandMine->SetOwner(this);
	}
}

