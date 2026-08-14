// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionBlueprintLibrary.h"
#include "GameFramework/Actor.h"
#include "GameplayActionInterface.h"
#include "GameplayActionComponent.h"
#include "GameplayActionTypes.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayActionEffect.h"
#include "GameplayActionFragment.h"

UGameplayActionComponent* UGameplayActionBlueprintLibrary::GetGameplayActionComponent(AActor* Actor)
{
	if (IsValid(Actor) && Actor->Implements<UGameplayActionInterface>())
	{
		return IGameplayActionInterface::Execute_GetGameplayActionComponent(Actor);
	}
	return nullptr;
}

FGameplayTagContainer UGameplayActionBlueprintLibrary::GetOwnedGameplayTagsFromActor(AActor* Actor)
{
	if (IsValid(Actor))
	{
		UGameplayActionComponent* Component = GetGameplayActionComponent(Actor);
		if (IsValid(Component))
		{
			return Component->GetOwnedGameplayTags();
		}
	}
	return FGameplayTagContainer();
}

bool UGameplayActionBlueprintLibrary::AddOwnedGameplayTagsToActor(AActor* Actor, FGameplayTagContainer Tags, int32 Quantity)
{
	if (IsValid(Actor))
	{
		UGameplayActionComponent* Component = GetGameplayActionComponent(Actor);
		if (IsValid(Component))
		{
			Component->AddOwnedGameplayTags(Tags, Quantity);
			return true;
		}
	}
	return false;
}

bool UGameplayActionBlueprintLibrary::RemoveOwnedGameplayTagsFromActor(AActor* Actor, FGameplayTagContainer Tags, int32 Quantity)
{
	if (IsValid(Actor))
	{
		UGameplayActionComponent* Component = GetGameplayActionComponent(Actor);
		if (IsValid(Component))
		{
			Component->RemoveOwnedGameplayTags(Tags, Quantity);
			return true;
		}
	}
	return false;
}

float UGameplayActionBlueprintLibrary::GetAttributeValueFromActor(AActor* Actor, const FGameplayActionAttribute& Attribute)
{
	if (IsValid(Actor))
	{
		UGameplayActionComponent* Component = GetGameplayActionComponent(Actor);
		if (IsValid(Component))
		{
			return Component->GetAttributeValue(Attribute);
		}
	}
	return 0.0f;
}

bool UGameplayActionBlueprintLibrary::EqualAttributeStruct(const FGameplayActionAttribute& Attribute1, const FGameplayActionAttribute& Attribute2)
{
    return Attribute1 == Attribute2;
}

bool UGameplayActionBlueprintLibrary::NotEqualAttributeStruct(const FGameplayActionAttribute &Attribute1, const FGameplayActionAttribute &Attribute2)
{
    return Attribute1 != Attribute2;
}

FGameplayActionEffectSpec UGameplayActionBlueprintLibrary::MakeGameplayEffectSpec(TSubclassOf<UGameplayActionEffect> Effect, int32 Level, AActor* SourceActor)
{
    if (!IsValid(Effect))
    {
        return FGameplayActionEffectSpec();
    }

    const UGameplayActionEffect* EffectCDO = Effect->GetDefaultObject<UGameplayActionEffect>();

    if (!IsValid(EffectCDO))
    {
        return FGameplayActionEffectSpec();
    }

    FGameplayActionEffectSpec Spec;
    Spec.Effect = EffectCDO;
	Spec.Level = Level;
	Spec.SourceActor = SourceActor;

    return Spec;
}

FGameplayActionEffectSpec UGameplayActionBlueprintLibrary::AddSetByCallerMagnitude(FGameplayActionEffectSpec Spec, FGameplayTag DataTag, float Magnitude)
{
	if (!IsValid(Spec.Effect) || !DataTag.IsValid())
	{
		return Spec;
	}

	Spec.SetSetByCallerMagnitude(DataTag, Magnitude);
	return Spec;
}

FActiveGameplayActionEffectHandle UGameplayActionBlueprintLibrary::ApplyGameplayEffectSpecToActor(AActor* Actor, const FGameplayActionEffectSpec& Spec)
{
    if (!IsValid(Actor) || !IsValid(Spec.Effect))
    {
        return FActiveGameplayActionEffectHandle();
    }

    UGameplayActionComponent* ActionComponent = GetGameplayActionComponent(Actor);

    if (!IsValid(ActionComponent))
    {
        return FActiveGameplayActionEffectHandle();
    }

    return ActionComponent->ApplyGameplayEffectSpecToSelf(Spec);
}

FActiveGameplayActionEffectHandle UGameplayActionBlueprintLibrary::ApplyGameplayEffectToActor(AActor* Actor, TSubclassOf<UGameplayActionEffect> Effect, int32 Level, AActor* SourceActor)
{
   if (!IsValid(Actor) || !IsValid(Effect))
   {
       return FActiveGameplayActionEffectHandle();
   }

   UGameplayActionComponent* ActionComponent = GetGameplayActionComponent(Actor);

   if (!IsValid(ActionComponent))
   {
       return FActiveGameplayActionEffectHandle();
   }

   return ActionComponent->ApplyGameplayEffectToSelf(Effect, Level, SourceActor);
}

UGameplayActionFragment* UGameplayActionBlueprintLibrary::FindFragment(
	const FGameplayActionFragmentsHandle& FragmentsHandle, TSubclassOf<UGameplayActionFragment> Fragment, bool& IsFound)
{
	IsFound = false;
	
	if (!IsValid(Fragment))
	{
		return nullptr;
	}
	
	const TObjectPtr<UGameplayActionFragment>* Found = FragmentsHandle.FragmentSlots.FindByPredicate([Fragment](const UGameplayActionFragment* FoundFragment)
	{
		return IsValid(FoundFragment) && FoundFragment->IsA(Fragment);
	});

	if (Found && IsValid(*Found))
	{
		IsFound = true;
		
		return *Found;
	}
	
	return nullptr;
}
