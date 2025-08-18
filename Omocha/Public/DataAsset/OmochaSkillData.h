// Copyright It's My Turn

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "OmochaSkillData.generated.h"

USTRUCT(BlueprintType)
struct OMOCHA_API FOmochaSkillData : public FTableRowBase
{
	GENERATED_BODY()
	
	// Damage
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback")
	bool bCanApplyKnockback = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback")
	float KnockbackForceMagnitude = 0.f;
    
	FOmochaSkillData()
	{
		BaseDamage = 3.f;
		DamageType = FGameplayTag();
		bCanApplyKnockback = false;
		KnockbackForceMagnitude = 0.f;
	}
};