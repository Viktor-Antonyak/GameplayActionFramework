// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionActorInfo.generated.h"

class UGameplayActionComponent;
class UMovementComponent;
class USkeletalMeshComponent;

/**
 * Struct that contains cached references to the actor's components.
 * This is useful for actions that need to access the actor's components frequently.
 */
USTRUCT(BlueprintType)
struct FGameplayActionActorInfo
{
	GENERATED_BODY()

	/** The actor that owns this Gameplay Action Component. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Actor Info")
	TWeakObjectPtr<AActor> OwnerActor;

	/** The Gameplay Action Component that owns this actor info. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Actor Info")
	TWeakObjectPtr<UGameplayActionComponent> ActionComponent;

	/** The movement component of the owner actor. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Actor Info")
	TWeakObjectPtr<UMovementComponent> MovementComponent;

	/** The skeletal mesh components of the owner actor. */
	TArray<TWeakObjectPtr<USkeletalMeshComponent>> SkeletalMeshComponents;
};
