#include "AbilitySystem/Abilities/OmochaHitReactAbility.h"

#include "AbilitySystemComponent.h"
#include "OmochaGameplayTags.h"
#include "Character/OmochaCharacterBase.h"
#include "GameFramework/Character.h"

UOmochaHitReactAbility::UOmochaHitReactAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer Tags;
	Tags.AddTag(FOmochaGameplayTags::Get().Effects_HitReact);
	SetAssetTags(Tags);

	AbilityTags.AddTag(FOmochaGameplayTags::Get().Effects_HitReact);
}

void UOmochaHitReactAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ApplyHitReactEffects();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(HitReactTimer,
		                                       [this, Handle, ActorInfo, ActivationInfo]()
		                                       {
			                                       EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		                                       },
		                                       HitReactDuration, false);
	}
}

void UOmochaHitReactAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                        bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveHitReactEffects();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitReactTimer);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UOmochaHitReactAbility::ApplyHitReactEffects()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FOmochaGameplayTags::Get().Effects_HitReact);
	}

	if (bApplyColorEffect && HitFlashMaterial)
	{
		if (AOmochaCharacterBase* Character = Cast<AOmochaCharacterBase>(GetAvatarActorFromActorInfo()))
		{
			Character->MulticastHitReact(HitFlashMaterial, HitReactDuration); 
		}
	}
}

void UOmochaHitReactAbility::RemoveHitReactEffects()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FOmochaGameplayTags::Get().Effects_HitReact);
	}
}