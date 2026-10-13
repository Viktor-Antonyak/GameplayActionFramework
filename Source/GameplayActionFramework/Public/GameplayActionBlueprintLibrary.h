// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayActionEffect.h"
#include "GameplayActionTypes.h"
#include "Templates/SubclassOf.h"
#include "GameplayActionBlueprintLibrary.generated.h"

class UGameplayActionFragment;
class UGameplayActionComponent;
struct FGameplayActionAttribute;

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
	UFUNCTION(BlueprintPure, Category = "Gameplay Tag")
	static FGameplayTagContainer GetOwnedGameplayTagsFromActor(AActor* Actor);

	/** Adds owned gameplay tags to the actor. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tag")
	static bool AddOwnedGameplayTagsToActor(AActor* Actor, FGameplayTagContainer Tags, int32 Quantity = 1);

	/** Removes owned gameplay tags from the actor. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tag")
	static bool RemoveOwnedGameplayTagsFromActor(AActor* Actor, FGameplayTagContainer Tags, int32 Quantity = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tag")
	static bool SendGameplayEventToActor(AActor* Actor, FGameplayTag EventTag);

	/** Returns the float value of an attribute from the actor. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	static float GetAttributeValueFromActor(AActor* Actor, const FGameplayActionAttribute& Attribute);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (==) Gameplay Attribute Struct", CompactNodeTitle = "==", AutoCreateRefTerm = "Attribute1, Attribute2"), Category = "Gameplay Attribute | Utilities")
	static bool EqualAttributeStruct(const FGameplayActionAttribute& Attribute1, const FGameplayActionAttribute& Attribute2);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (!=) Gameplay Attribute Struct", CompactNodeTitle = "!=", AutoCreateRefTerm = "Attribute1, Attribute2"), Category = "Gameplay Attribute | Utilities")
	static bool NotEqualAttributeStruct(const FGameplayActionAttribute& Attribute1, const FGameplayActionAttribute& Attribute2);

	/** Creates a gameplay effect spec from an effect class. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	static FGameplayActionEffectSpec MakeGameplayEffectSpec(TSubclassOf<UGameplayActionEffect> Effect, int32 Level = 1, AActor* SourceActor = nullptr);

	/** Adds a set-by-caller magnitude to the effect spec. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	static FGameplayActionEffectSpec AddSetByCallerMagnitude(FGameplayActionEffectSpec Spec, FGameplayTag DataTag, float Magnitude);
	
	/** Applies a gameplay effect to the target actor by spec. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	static FActiveGameplayActionEffectHandle ApplyGameplayEffectSpecToActor(AActor* Actor, const FGameplayActionEffectSpec& Spec);
	
	/** Applies a gameplay effect to the target actor by effect class. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	static FActiveGameplayActionEffectHandle ApplyGameplayEffectToActor(AActor* Actor, TSubclassOf<UGameplayActionEffect> Effect, int32 Level = 1, AActor* SourceActor = nullptr);
	
	/** Finds a fragment of the given class inside the fragments handle. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Fragments", meta = (DeterminesOutputType = "Fragment", ExpandBoolAsExecs = "IsFound"))
	static UGameplayActionFragment* FindFragment(const FGameplayActionFragmentsHandle& FragmentsHandle,
											  TSubclassOf<UGameplayActionFragment> Fragment, bool& IsFound);
};
