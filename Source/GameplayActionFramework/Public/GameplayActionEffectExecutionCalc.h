// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GameplayActionEffectExecutionCalc.generated.h"

struct FGameplayActionEffectSpec;
class UGameplayActionComponent;

/** Base class for custom gameplay effect execution. Override Execute in Blueprint for full control over effect application. */
UCLASS(Blueprintable, Abstract)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionEffectExecutionCalc : public UObject
{
	GENERATED_BODY()

public:
	/** Executes custom effect logic. Replaces the standard modifier loop when set on a gameplay effect. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Effect Execution")
	void Execute(const FGameplayActionEffectSpec& EffectSpec, UGameplayActionComponent* Target);

	void Execute_Implementation(const FGameplayActionEffectSpec& EffectSpec, UGameplayActionComponent* Target) {}
};
