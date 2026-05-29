// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionBlueprintLibrary.h"
#include "GameFramework/Actor.h"
#include "GameplayActionInterface.h"
#include "GameplayActionComponent.h"
#include "GameplayActionSpec.h"
#include "GameplayAttributeSet.h"
#include "GameplayEffect.h"
#include "UObject/Object.h"

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

bool UGameplayActionBlueprintLibrary::AddOwnedGameplayTagsToActor(AActor* Actor, FGameplayTagContainer Tags)
{
	if (IsValid(Actor))
	{
		UGameplayActionComponent* Component = GetGameplayActionComponent(Actor);
		if (IsValid(Component))
		{
			Component->AddOwnedGameplayTags(Tags);
			return true;
		}
	}
	return false;
}

bool UGameplayActionBlueprintLibrary::RemoveOwnedGameplayTagsFromActor(AActor* Actor, FGameplayTagContainer Tags)
{
	if (IsValid(Actor))
	{
		UGameplayActionComponent* Component = GetGameplayActionComponent(Actor);
		if (IsValid(Component))
		{
			Component->RemoveOwnedGameplayTags(Tags);
			return true;
		}
	}
	return false;
}

float UGameplayActionBlueprintLibrary::GetAttributeValueFromActor(AActor* Actor, const FGameplayAttribute& Attribute)
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

bool UGameplayActionBlueprintLibrary::EqualAttributeStruct(const FGameplayAttribute& Attribute1, const FGameplayAttribute& Attribute2)
{
    return Attribute1 == Attribute2;
}

bool UGameplayActionBlueprintLibrary::NotEqualAttributeStruct(const FGameplayAttribute &Attribute1, const FGameplayAttribute &Attribute2)
{
    return Attribute1 != Attribute2;
}

FGameplayEffectSpec UGameplayActionBlueprintLibrary::MakeGameplayEffectSpec(TSubclassOf<UGameplayEffect> Effect)
{
    if (!IsValid(Effect))
    {
        return FGameplayEffectSpec();
    }

    const UGameplayEffect* EffectCDO = Effect->GetDefaultObject<UGameplayEffect>();

    if (!IsValid(EffectCDO))
    {
        return FGameplayEffectSpec();
    }

    FGameplayEffectSpec Spec;
    Spec.Effect = EffectCDO;

    return Spec;
}

FGameplayEffectSpec UGameplayActionBlueprintLibrary::AddSetByCallerMagnitude(FGameplayEffectSpec Spec, FGameplayTag DataTag, float Magnitude)
{
    if (!IsValid(Spec.Effect) || !DataTag.IsValid())
    {
        return Spec;
    }

    FGameplayEffectSpec NewSpec = Spec;
    NewSpec.SetSetByCallerMagnitude(DataTag, Magnitude);

    return NewSpec;
}

FActiveGameplayEffectHandle UGameplayActionBlueprintLibrary::ApplyGameplayEffectSpecToActor(AActor* Actor, const FGameplayEffectSpec& Spec)
{
    if (!IsValid(Actor) || !IsValid(Spec.Effect))
    {
        return FActiveGameplayEffectHandle();
    }

    UGameplayActionComponent* ActionComponent = GetGameplayActionComponent(Actor);

    if (!IsValid(ActionComponent))
    {
        return FActiveGameplayEffectHandle();
    }

    return ActionComponent->ApplyGameplayEffectSpecToSelf(Spec);
}

FActiveGameplayEffectHandle UGameplayActionBlueprintLibrary::ApplyGameplayEffectToActor(AActor* Actor, TSubclassOf<UGameplayEffect> Effect)
{
   if (!IsValid(Actor) || !IsValid(Effect))
   {
       return FActiveGameplayEffectHandle();
   }

   UGameplayActionComponent* ActionComponent = GetGameplayActionComponent(Actor);

   if (!IsValid(ActionComponent))
   {
       return FActiveGameplayEffectHandle();
   }

   return ActionComponent->ApplyGameplayEffectToSelf(Effect);
}
