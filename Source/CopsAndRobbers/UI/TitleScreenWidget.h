// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleScreenWidget.generated.h"


UCLASS()
class COPSANDROBBERS_API UTitleScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	/* 마우스 클릭 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/* 키보드 클릭 */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LevelToLoad;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* AnimToWidget;

	bool bIsLoadingLevel = false;

	void LoadGameLevel();
};
