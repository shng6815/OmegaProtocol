// Copyright It's My Turn


#include "Actor/Shield.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "OmochaGameplayTags.h"
#include "AbilitySystem/OmochaAbilitySystemComponent.h"
#include "AbilitySystem/OmochaAbilitySystemLibrary.h"
#include "AbilitySystem/OmochaShieldAttributeSet.h"
#include "Character/OmochaCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Omocha/Omocha.h"


AShield::AShield()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UOmochaAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UOmochaShieldAttributeSet>(TEXT("AttributeSet"));

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetGenerateOverlapEvents(true);
	Box->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBoxOverlap);

	Shield = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Shield"));
	Shield->SetupAttachment(Box);
	Shield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Shield->SetCollisionResponseToAllChannels(ECR_Ignore);
	Shield->SetVisibility(false);
}

void AShield::SetDamageEffectParams(const FDamageEffectParams& DamageEffectParams)
{
	DamageParams = DamageEffectParams;
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		DashingTimerHandle,
		this,
		&AShield::StopDashing,
		0.2f,
		false);
	bDashing = true;
}

void AShield::BroadCastInitialValues()
{
	if (IsValid(OwnerCharacter))
	{
		if (UOmochaAbilitySystemComponent* ASC = Cast<UOmochaAbilitySystemComponent>(OwnerCharacter->GetAbilitySystemComponent()))
		{
			if (UOmochaShieldAttributeSet* SAS = Cast<UOmochaShieldAttributeSet>(AttributeSet))
			{
				FOnAttributeChangeData Health;
				Health.NewValue = SAS->GetHealth();
				ASC->BroadcastShieldHealthChanged(Health);
				Health.NewValue = SAS->GetMaxHealth();
				ASC->BroadcastShieldMaxHealthChanged(Health);
			}
		}
	}
}

void AShield::BindShieldHealthAttributes()
{
	UOmochaShieldAttributeSet* AS = CastChecked<UOmochaShieldAttributeSet>(AttributeSet);
	if (IsValid(OwnerCharacter))
	{
		if (UOmochaAbilitySystemComponent* ASC = Cast<UOmochaAbilitySystemComponent>(OwnerCharacter->GetAbilitySystemComponent()))
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute())
			.AddUObject(ASC, &UOmochaAbilitySystemComponent::BroadcastShieldHealthChanged);
			
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute())
			.AddUObject(ASC, &UOmochaAbilitySystemComponent::BroadcastShieldMaxHealthChanged);
		}
	}
}

void AShield::BeginPlay()
{
	Super::BeginPlay();
}

void AShield::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OwnerCharacter) && HasAuthority() && bDashing && OwnerCharacter != OtherActor)
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			DamageParams.TargetAbilitySystemComponent = TargetASC;
			UOmochaAbilitySystemLibrary::ApplyDamageEffect(DamageParams);
		}
	}
}

void AShield::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAttributes(1);
	BindCallbacks();
}

void AShield::InitializeAttributes(int32 Level) const
{
	if (bAppliedAttributeEffect) return;
	
	check(IsValid(GetAbilitySystemComponent()));
	check(ShieldAttributes);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		ShieldAttributes, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

bool AShield::Equip()
{
	UOmochaAbilitySystemComponent* ASC = CastChecked<UOmochaAbilitySystemComponent>(AbilitySystemComponent);
	UOmochaShieldAttributeSet* AS = CastChecked<UOmochaShieldAttributeSet>(AttributeSet);

	if (ASC->HasMatchingGameplayTag(FOmochaGameplayTags::Get().Shield_Status_Broken)
		|| AS->GetHealth() <= 0.f)
	{
		return false;
	}

	check(Box);
	check(Shield);

	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Overlap);
	Box->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Box->SetCollisionResponseToChannel(ECC_EQSTrace, ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_MoveTrace, ECR_Ignore);

	if (OwnerCharacter->ActorHasTag(FName("Player")))
	{
		Box->SetCollisionResponseToChannel(ECC_ChainTrace, ECR_Ignore);
		Box->SetCollisionResponseToChannel(ECC_EnemyChainTrace, ECR_Block);
	}
	else if (OwnerCharacter->ActorHasTag(FName("Enemy")))
	{
		Box->SetCollisionResponseToChannel(ECC_ChainTrace, ECR_Block);
		Box->SetCollisionResponseToChannel(ECC_EnemyChainTrace, ECR_Ignore);
	}
	else if (OwnerCharacter->ActorHasTag(FName("Fight")))
	{
		Box->SetCollisionResponseToChannel(ECC_ChainTrace, ECR_Block);
		Box->SetCollisionResponseToChannel(ECC_EnemyChainTrace, ECR_Block);
	}


	Shield->SetVisibility(true);

	ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(FOmochaGameplayTags::Get().Shield_Status_Timer));
	ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(FOmochaGameplayTags::Get().Shield_Status_Repaired));

	return true;
}

bool AShield::Unequip()
{
	check(Box);
	check(Shield);

	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Shield->SetVisibility(false);
	bDashing = false;

	UOmochaAbilitySystemComponent* ASC = CastChecked<UOmochaAbilitySystemComponent>(AbilitySystemComponent);
	if (ASC->HasMatchingGameplayTag(FOmochaGameplayTags::Get().Shield_Status_Broken))
	{
		return false;
	}
	ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(FOmochaGameplayTags::Get().Shield_Status_Using));
	Repair();

	return true;
}

void AShield::SetOwnerCharacter(UObject* InOwnerCharacter)
{
	OwnerCharacter = Cast<AOmochaCharacterBase>(InOwnerCharacter);
}

ACharacter* AShield::GetOwnerCharacter()
{
	return OwnerCharacter;
}

void AShield::Broken()
{
	check(Box);
	check(Shield);

	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Shield->SetVisibility(false);
	bDashing = false;

	UOmochaAbilitySystemComponent* ASC = CastChecked<UOmochaAbilitySystemComponent>(AbilitySystemComponent);
	if (ASC->HasMatchingGameplayTag(FOmochaGameplayTags::Get().Shield_Status_Broken))
	{
		return;
	}

	check(ShieldBrokenEffectClass);
	check(ShieldRepairedEffectClass);

	ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(FOmochaGameplayTags::Get().Shield_Status_Using));

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	FActiveGameplayEffectHandle BrokenTimerHandle = ASC->ApplyGameplayEffectToSelf(
		Cast<UGameplayEffect>(ShieldBrokenEffectClass->GetDefaultObject()),
		1.0f,
		EffectContext);

	if (BrokenTimerHandle.IsValid())
	{
		if (FOnActiveGameplayEffectRemoved_Info* OnEffectRemovedDelegate = ASC->OnGameplayEffectRemoved_InfoDelegate(
			BrokenTimerHandle))
		{
			OnEffectRemovedDelegate->AddLambda(
				[this](const FGameplayEffectRemovalInfo& Effect)
				{
					this->Repair();
				}
			);
		}
	}

	OnShieldDestroyed.Broadcast();
}

void AShield::Damaged()
{
	OnShieldDamaged.Broadcast();
}

void AShield::Repair()
{
	UOmochaAbilitySystemComponent* ASC = CastChecked<UOmochaAbilitySystemComponent>(AbilitySystemComponent);
	if (ASC->HasMatchingGameplayTag(FOmochaGameplayTags::Get().Shield_Status_Using))
	{
		return;
	}

	check(ShieldTimerEffectClass);
	check(ShieldRepairedEffectClass);

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	FActiveGameplayEffectHandle TimerHandle = ASC->ApplyGameplayEffectToSelf(
		Cast<UGameplayEffect>(ShieldTimerEffectClass->GetDefaultObject()),
		1.0f,
		EffectContext);

	if (TimerHandle.IsValid())
	{
		if (FOnActiveGameplayEffectRemoved_Info* OnEffectRemovedDelegate = ASC->
			OnGameplayEffectRemoved_InfoDelegate(TimerHandle))
		{
			OnEffectRemovedDelegate->AddLambda(
				[this](const FGameplayEffectRemovalInfo& Effect)
				{
					if (this->AbilitySystemComponent->HasMatchingGameplayTag(
						FOmochaGameplayTags::Get().Shield_Status_Using))
					{
						return;
					}

					FGameplayEffectContextHandle RpairContext = this->AbilitySystemComponent->MakeEffectContext();
					this->AbilitySystemComponent->ApplyGameplayEffectToSelf(
						Cast<UGameplayEffect>(this->ShieldRepairedEffectClass->GetDefaultObject()),
						1.0f,
						RpairContext);
				}
			);
		}
	}
}


void AShield::BindCallbacks()
{
	UOmochaShieldAttributeSet* AS = CastChecked<UOmochaShieldAttributeSet>(AttributeSet);
	AS->OnShieldBroken.AddUObject(this, &AShield::Broken);
	AS->OnShieldDamaged.AddUObject(this, &AShield::Damaged);
}

void AShield::StopDashing()
{
	bDashing = false;
}
