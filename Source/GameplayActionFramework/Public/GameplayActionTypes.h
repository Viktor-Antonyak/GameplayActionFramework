// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTypes.generated.h"

class UGameplayActionModule;
class UGameplayActionComponent;
class UMovementComponent;
class UGameplayAction;
class UGameplayActionFragment;

/** Spec for a gameplay action added to the component. */
USTRUCT(BlueprintType)
struct FGameplayActionSpec
{
	GENERATED_BODY()
	
	/** The gameplay action instance. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Spec")
	UGameplayAction* Action = nullptr;
	
	/** Action power level. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Spec")
	int32 Level = -1;
	
	/** Input binding ID (copied from the action class). */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Spec")
	int32 InputID = -1;

	/** Returns whether the action pointer is valid. */
	bool IsValid() const
	{
		return Action != nullptr;
	}
	
	bool operator==(const FGameplayActionSpec& Other) const
	{
		return Action == Other.Action && Level == Other.Level && InputID == Other.InputID;
	}
	
	bool operator!=(const FGameplayActionSpec& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Struct that contains cached references to the actor's components.
 * This is useful for actions that need to access the actor's components frequently.
 */
USTRUCT(BlueprintType)
struct FGameplayActionActorInfo
{
	GENERATED_BODY()
	
	/** The actor that owns this Gameplay Action Component. */
	TWeakObjectPtr<AActor> OwnerActor;

	/** The Gameplay Action Component that owns this actor info. */
	TWeakObjectPtr<UGameplayActionComponent> ActionComponent;

	/** The movement component of the owner actor. */
	TWeakObjectPtr<UMovementComponent> MovementComponent;

	/** The skeletal mesh components of the owner actor. */
	TArray<TWeakObjectPtr<USkeletalMeshComponent>> SkeletalMeshComponents;
};

/** 
 * Struct that holds Gameplay Action Fragments.
 * Use FindFragment() to check if specific Fragment exist inside handle and get optional data.
 */
USTRUCT(BlueprintType)
struct FGameplayActionFragmentsHandle
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Instanced, Category = "Gameplay Action Fragments Handle", meta=(ShowOnlyInnerProperties))
	TArray<TObjectPtr<UGameplayActionFragment>> FragmentSlots;
};
