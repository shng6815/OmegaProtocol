// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UI/WidgetController/OmochaWidgetController.h"
#include "OverlayWidgetController.generated.h"

/**
 * 
 */
class UOmochaUserWidget;
class UOmochaAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewValue);

UCLASS(BlueprintType, Blueprintable)
class OMOCHA_API UOverlayWidgetController : public UOmochaWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadCastInitialValues();
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnHealthChangedSignature OnShieldHealthChanged;

	UPROPERTY(BlueprintAssignable, Category ="GAS|Attributes")
	FOnHealthChangedSignature OnShieldMaxHealthChanged;

protected:
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void ShieldHealthChanged(const float NewValue) const;
	void ShieldMaxHealthChanged(const float NewValue) const;
};
