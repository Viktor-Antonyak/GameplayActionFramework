// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GameplayAttributeSet.h"
#include "GameplayEffect.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectMacros.h"
#include "GameplayActionBlueprintLibrary.generated.h"

class UGameplayActionComponent;
struct FGameplayAttribute;

/**
 * Blueprint function library for the Gameplay Action Framework.
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns the Gameplay Action Component of the actor. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	static UGameplayActionComponent* GetGameplayActionComponent(AActor* Actor);

	/** Returns the owned gameplay tags of the actor. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	static FGameplayTagContainer GetOwnedGameplayTagsFromActor(AActor* Actor);

	/** Adds owned gameplay tags to the actor. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	static bool AddOwnedGameplayTagsToActor(AActor* Actor, FGameplayTagContainer Tags);

	/** Removes owned gameplay tags from the actor. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	static bool RemoveOwnedGameplayTagsFromActor(AActor* Actor, FGameplayTagContainer Tags);

	/** Returns the float value of an attribute from the actor. */
	UFUNCTION(BlueprintCallable, Category ="Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	static float GetAttributeValueFromActor(AActor* Actor, const FGameplayAttribute& Attribute);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (GameplayAttributeStruct)", CompactNodeTitle = "==", AutoCreateRefTerm = "Attribute1, Attribute2"), Category = "Gameplay Attribute | Utilities")
	static bool EqualAttributeStruct(const FGameplayAttribute& Attribute1, const FGameplayAttribute& Attribute2);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (GameplayAttributeStruct)", CompactNodeTitle = "!=", AutoCreateRefTerm = "Attribute1, Attribute2"), Category = "Gameplay Attribute | Utilities")
	static bool NotEqualAttributeStruct(const FGameplayAttribute& Attribute1, const FGameplayAttribute& Attribute2);

	/** Creates a gameplay effect spec from an effect class. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	static FGameplayEffectSpec MakeGameplayEffectSpec(TSubclassOf<UGameplayEffect> Effect);

	/** Adds a set-by-caller magnitude to the effect spec. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	static FGameplayEffectSpec AddSetByCallerMagnitude(FGameplayEffectSpec Spec, FGameplayTag DataTag, float Magnitude);
	
	/** Applies a gameplay effect to the target actor by spec. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	static FActiveGameplayEffectHandle ApplyGameplayEffectSpecToActor(AActor* Actor, const FGameplayEffectSpec& Spec);
	
	/** Applies a gameplay effect to the target actor by effect class. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	static FActiveGameplayEffectHandle ApplyGameplayEffectToActor(AActor* Actor, TSubclassOf<UGameplayEffect> Effect);
};
