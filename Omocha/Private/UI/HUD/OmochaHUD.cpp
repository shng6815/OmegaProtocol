// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/OmochaHUD.h"

#include "Character/OmochaPlayerCharacter.h"
#include "GameFramework/PlayerState.h"
#include "Player/OmochaPlayerState.h"
#include "UI/Widget/OmochaUserWidget.h"
#include "UI/Widget/StatusWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/StatWidgetController.h"

UOverlayWidgetController* AOmochaHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void AOmochaHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS,
                             AOmochaPlayerCharacter* AC)
{
	checkf(OverlayWidgetClass, TEXT("Overaly WC uninitialized"));
	checkf(OverlayWidgetControllerClass, TEXT("Overaly WCC uninitialized"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UOmochaUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS, AC);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	OverlayWidget->AddToViewport();
	WidgetController->BroadCastInitialValues();
}

UStatWidgetController* AOmochaHUD::GetStatWidgetController(const FWidgetControllerParams& WCParams)
{
	if (StatWidgetController == nullptr)
	{
		StatWidgetController = NewObject<UStatWidgetController>(this, StatWidgetControllerClass);
		StatWidgetController->SetWidgetControllerParams(WCParams);
		StatWidgetController->BindCallbacksToDependencies();
	}
	return StatWidgetController;
}

void AOmochaHUD::ShowStatWidget()
{
	if (!StatWidgetClass || !StatWidgetControllerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("StatWidgetClass or StatWidgetControllerClass is not set in AOmochaHUD!"));
		return;
	}

	if (StatWidget == nullptr)
	{
		StatWidget = CreateWidget<UStatusWidget>(GetWorld(), StatWidgetClass);
		if (!StatWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create StatWidget!"));
			return;
		}
		APlayerController* PC = GetOwningPlayerController();
		if (!IsValid(PC))
		{
			return;
		}
		APlayerState* PS = PC->GetPlayerState<APlayerState>();
		if (!IsValid(PS))
		{
			return;
		}
		UAbilitySystemComponent* ASC = Cast<AOmochaPlayerState>(PS)->GetAbilitySystemComponent();
		if (!IsValid(ASC))
		{
			return;
		}
		UAttributeSet* AS = Cast<AOmochaPlayerState>(PS)->GetAttributeSet();
		if (!IsValid(AS))
		{
			return;
		}
		AOmochaPlayerCharacter* AC = Cast<AOmochaPlayerCharacter>(PC->GetPawn());
		const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS, AC);

		UStatWidgetController* WidgetController = GetStatWidgetController(WidgetControllerParams);
		StatWidget->SetWidgetController(WidgetController);

		WidgetController->BroadCastInitialValues();

		StatWidget->AddToViewport();
		StatWidget->PlayAppearAnimation();
	}
	else
	{
		StatWidget->PlayAppearAnimation();
	}
}

void AOmochaHUD::HideStatWidget()
{
	if (UStatusWidget* OmochaWidget = Cast<UStatusWidget>(StatWidget))
	{
		OmochaWidget->PlayDisappearAnimation();
	}
}
