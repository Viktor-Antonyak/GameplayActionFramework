// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "ActiveGameplayEffectsContainer.h"
#include "GameFramework/Actor.h"

FActiveGameplayEffect* FActiveGameplayEffectsContainer::Find(const FActiveGameplayEffectHandle& Handle)
{
	return Effects.Find(Handle);
}

const FActiveGameplayEffect* FActiveGameplayEffectsContainer::Find(const FActiveGameplayEffectHandle& Handle) const
{
	return Effects.Find(Handle);
}

FActiveGameplayEffectHandle FActiveGameplayEffectsContainer::FindStackable(const UGameplayEffect* EffectCDO, const AActor* SourceActor) const
{
	if (!EffectCDO || EffectCDO->StackingPolicy == EGameplayEffectStacking::None)
	{
		return FActiveGameplayEffectHandle();
	}

	for (const TTuple<FActiveGameplayEffectHandle, FActiveGameplayEffect>& Pair : Effects)
	{
		const FActiveGameplayEffect& ExistingEffect = Pair.Value;

		if (ExistingEffect.Spec.Effect != EffectCDO)
		{
			continue;
		}

		if (EffectCDO->StackingPolicy == EGameplayEffectStacking::AggregateBySource)
		{
			if (ExistingEffect.Spec.SourceActor.Get() != SourceActor)
			{
				continue;
			}
		}
		
		return Pair.Key;
	}

	return FActiveGameplayEffectHandle();
}

FActiveGameplayEffectHandle FActiveGameplayEffectsContainer::Add(const FActiveGameplayEffect& NewEffect)
{
	if (!NewEffect.Handle.IsValid())
	{
		return FActiveGameplayEffectHandle();
	}

	Effects.Add(NewEffect.Handle, NewEffect);
	
	return NewEffect.Handle;
}

bool FActiveGameplayEffectsContainer::Remove(const FActiveGameplayEffectHandle& Handle)
{
	return Effects.Remove(Handle) > 0;
}
