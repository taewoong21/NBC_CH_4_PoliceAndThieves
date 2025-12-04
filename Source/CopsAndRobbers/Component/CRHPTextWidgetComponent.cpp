// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CRHPTextWidgetComponent.h"

#include "UI/UW_HPText.h"

void UCRHPTextWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UUW_HPText* HPText = Cast<UUW_HPText>(GetWidget());
	if (IsValid(HPText) == true)
	{
		HPText->SetOwningActor(GetOwner());
	}
}
