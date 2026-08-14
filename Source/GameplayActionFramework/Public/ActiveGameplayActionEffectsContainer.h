// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionEffect.h"
#include "ActiveGameplayActionEffectsContainer.generated.h"

class AActor;

USTRUCT()
struct FActiveGameplayActionEffectsContainer
{
	GENERATED_BODY()
	
public:
	
	/** Returns the active effect for the handle, or nullptr if the handle is invalid or absent. */
	FActiveGameplayActionEffect* Find(const FActiveGameplayActionEffectHandle& Handle);
	
	const FActiveGameplayActionEffect* Find(const FActiveGameplayActionEffectHandle& Handle) const;
	
	/** Returns the handle of an existing stackable effect of the same class and source, or an invalid handle. */
	FActiveGameplayActionEffectHandle FindStackable(const UGameplayActionEffect* EffectCDO, const AActor* SourceActor) const;
	
	/** Adds the effect to the container and returns its new handle. */
	FActiveGameplayActionEffectHandle Add(const FActiveGameplayActionEffect& NewEffect);
	
	/** Removes the effect for the handle; returns true if it existed. */
	bool Remove(const FActiveGameplayActionEffectHandle& Handle);
	
	void Empty()
	{
		Effects.Empty();
	}
	
	const TMap<FActiveGameplayActionEffectHandle, FActiveGameplayActionEffect>& GetAll() const
	{
		return Effects;
	}
	
private:
	
	UPROPERTY()
	TMap<FActiveGameplayActionEffectHandle, FActiveGameplayActionEffect> Effects;
};
