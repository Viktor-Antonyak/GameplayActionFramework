// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionEffectTypes.h"
#include "GameplayActionAttributeAggregator.generated.h"


/** A single modifier contribution from one active effect, tracked so it can be precisely removed later. */
USTRUCT()
struct FAggregatorModifier
{
	GENERATED_BODY()

	UPROPERTY()
	FActiveGameplayActionEffectHandle SourceHandle;

	UPROPERTY()
	EGameplayActionEffectModOp ModifierOp = EGameplayActionEffectModOp::Additive;

	UPROPERTY()
	float Magnitude = 0.0f;
};

/**
 * Tracks all active (non-instant) modifiers contributing to a single attribute, and recomputes
 * the attribute's final value from BaseValue + all active modifiers, rather than mutating it in place.
 * This allows overlapping effects on the same attribute to be added/removed independently and correctly,
 * regardless of application order.
 */
USTRUCT()
struct FGameplayActionAttributeAggregator
{
	GENERATED_BODY()

public:
	
	/** Adds a new modifier contribution, tagged with the effect handle that owns it. */
	void AddModifier(const FActiveGameplayActionEffectHandle& SourceHandle, EGameplayActionEffectModOp Op, float Magnitude);

	/** Removes all modifiers contributed by a specific effect handle. Returns the number of modifiers removed. */
	int32 RemoveModifiersFromSource(const FActiveGameplayActionEffectHandle& SourceHandle);

	/** Recomputes the final value: (BaseValue + Sum(Additive)) * Product(Multiplicative), or the last Override if any exist. */
	float CalculateCurrentValue(float BaseValue) const;

	bool IsEmpty() const { return Modifiers.Num() == 0; }

	int32 Num() const { return Modifiers.Num(); }

private:
	
	UPROPERTY()
	TArray<FAggregatorModifier> Modifiers;
};