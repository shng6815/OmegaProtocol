// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/OmochaAbilitySystemComponent.h"
#include "AbilitySystem/OmochaAttributeSet.h"
#include "Player/OmochaPlayerController.h"

void UOverlayWidgetController::BroadCastInitialValues()
{
	const UOmochaAttributeSet* OmochaAttributeSet =  CastChecked<UOmochaAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(OmochaAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(OmochaAttributeSet->GetMaxHealth());
	BroadcastAbilityInfo();
	if (PlayerController)
	{
		PlayerController->SetCursorEnabled(true);
	}
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const UOmochaAttributeSet* OmochaAttributeSet =  CastChecked<UOmochaAttributeSet>(AttributeSet);
	if (!IsValid(AbilitySystemComponent)) return;
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		OmochaAttributeSet->GetHealthAttribute())
		.AddUObject(this, &UOverlayWidgetController::HealthChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		OmochaAttributeSet->GetMaxHealthAttribute())
		.AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);
	
	AbilitySystemComponent->ShieldHealthChangedDelegate.AddUObject(this, &UOverlayWidgetController::ShieldHealthChanged);

	AbilitySystemComponent->ShieldMaxHealthChangedDelegate.AddUObject(this, &UOverlayWidgetController::ShieldMaxHealthChanged);
	
	AbilitySystemComponent->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);

	AbilitySystemComponent->AbilitiesSpecsGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::ShieldHealthChanged(const float NewValue) const
{
	OnShieldHealthChanged.Broadcast(NewValue);
}

void UOverlayWidgetController::ShieldMaxHealthChanged(const float NewValue) const
{
	OnShieldMaxHealthChanged.Broadcast(NewValue);
}
