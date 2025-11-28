// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleScreenWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Input/Events.h"

/*  */
void UTitleScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsLoadingLevel = false;

	if (AnimToWidget != nullptr)
	{
		PlayAnimation(AnimToWidget, 0.0f, 0, EUMGSequencePlayMode::Forward, 1.0f); // 깜빡임 반복 재생
	}
	
	bIsFocusable = true; // 키보드 입력 활성화
}

/* 마우스 클릭 이벤트 */
FReply UTitleScreenWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) // 좌클릭 시, 맵 로드
	{
		LoadGameLevel();

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

/* 키보드 입력 이벤트 */
FReply UTitleScreenWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	/* 아무 키 입력이든 맵 로드 */
	LoadGameLevel();

	return FReply::Handled();
}

void UTitleScreenWidget::LoadGameLevel()
{
	if (bIsLoadingLevel) return; // 연속 로딩 방지

	if (LevelToLoad.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[TitleScreenWidget] Level not set in Title Screen Widget."));
		return;
	}

	bIsLoadingLevel = true;

	/* 플레이 맵 열기 */
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(LevelToLoad.ToString()));
	UGameplayStatics::OpenLevel(this, LevelName);
}
