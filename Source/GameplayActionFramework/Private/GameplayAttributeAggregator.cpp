// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAttributeAggregator.h"

void FGameplayAttributeAggregator::AddModifier(const FActiveGameplayEffectHandle& SourceHandle, EGameplayModOp Op, float Magnitude)
{
	FAggregatorModifier NewModifier;
	NewModifier.SourceHandle = SourceHandle;
	NewModifier.ModifierOp = Op;
	NewModifier.Magnitude = Magnitude;

	Modifiers.Add(NewModifier);
}

int32 FGameplayAttributeAggregator::RemoveModifiersFromSource(const FActiveGameplayEffectHandle& SourceHandle)
{
	return Modifiers.RemoveAll([&SourceHandle](const FAggregatorModifier& Mod)
	{
		return Mod.SourceHandle == SourceHandle;
	});
}

float FGameplayAttributeAggregator::CalculateCurrentValue(float BaseValue) const
{
	float AdditiveSum = 0.0f;
	
	float MultiplicativeProduct = 1.0f;
	
	bool bHasOverride = false;
	
	float OverrideValue = 0.0f;

	for (const FAggregatorModifier& Mod : Modifiers)
	{
		switch (Mod.ModifierOp)
		{
		case EGameplayModOp::Additive:
			AdditiveSum += Mod.Magnitude;
			break;

		case EGameplayModOp::Multiplicative:
			MultiplicativeProduct *= Mod.Magnitude;
			break;

		case EGameplayModOp::Override:
			bHasOverride = true;
			OverrideValue = Mod.Magnitude;
			break;
		}
	}

	if (bHasOverride)
	{
		return OverrideValue;
	}

	return (BaseValue + AdditiveSum) * MultiplicativeProduct;
}