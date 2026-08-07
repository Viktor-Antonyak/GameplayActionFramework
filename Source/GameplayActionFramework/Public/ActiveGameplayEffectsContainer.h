// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ActiveGameplayEffectsContainer.generated.h"

class AActor;

USTRUCT()
struct FActiveGameplayEffectsContainer
{
	GENERATED_BODY()
	
public:
	
	FActiveGameplayEffect* Find(const FActiveGameplayEffectHandle& Handle);
	
	const FActiveGameplayEffect* Find(const FActiveGameplayEffectHandle& Handle) const;
	
	FActiveGameplayEffectHandle FindStackable(const UGameplayEffect* EffectCDO, const AActor* SourceActor) const;
	
	FActiveGameplayEffectHandle Add(const FActiveGameplayEffect& NewEffect);
	
	bool Remove(const FActiveGameplayEffectHandle& Handle);
	
	void Empty()
	{
		Effects.Empty();
	}
	
	const TMap<FActiveGameplayEffectHandle, FActiveGameplayEffect>& GetAll() const
	{
		return Effects;
	}
	
private:
	
	UPROPERTY()
	TMap<FActiveGameplayEffectHandle, FActiveGameplayEffect> Effects;
};
