// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_HPText.h"

#include "Component/CRStatusComponent.h"
#include "Components/TextBlock.h"
#include "Player/CRCharacter.h"

UUW_HPText::UUW_HPText(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UUW_HPText::NativeConstruct()
{
	Super::NativeConstruct();

	ACRCharacter* OwningCharacter = Cast<ACRCharacter>(OwningActor);
	if (IsValid(OwningCharacter) == true)
	{
		OwningCharacter->SetHPTextWidget(this);
	}
}

void UUW_HPText::InitializeHPTextWidget(UCRStatusComponent* InStatusComponent)
{
	OnMaxHPChange(InStatusComponent->GetMaxHP());
	OnCurrentHPChange(InStatusComponent->GetCurrentHP());
}

void UUW_HPText::OnMaxHPChange(float InMaxHP)
{
	MaxHPText->SetText(FText::FromString(FString::SanitizeFloat(InMaxHP)));
}

void UUW_HPText::OnCurrentHPChange(float InCurrentHP)
{
	CurrentHPText->SetText(FText::FromString(FString::SanitizeFloat(InCurrentHP)));
}
