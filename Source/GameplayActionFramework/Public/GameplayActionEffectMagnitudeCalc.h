// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GameplayActionEffectMagnitudeCalc.generated.h"

struct FGameplayActionEffectSpec;
class UGameplayActionComponent;

/** 
 * Base class for gameplay effect magnitude calculations. Override CalculateMagnitude in Blueprint. 
 */
UCLASS(Blueprintable, Abstract)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionEffectMagnitudeCalc : public UObject
{
	GENERATED_BODY()

public:

    /** Calculates the magnitude of the given Gameplay Effect Spec. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Effect Magnitude Calculation")
	float CalculateMagnitude(const FGameplayActionEffectSpec& EffectSpec, UGameplayActionComponent* ActionComponent);

	float CalculateMagnitude_Implementation(const FGameplayActionEffectSpec& EffectSpec, UGameplayActionComponent* ActionComponent)
	{
		return 0.0f;
	}
};
