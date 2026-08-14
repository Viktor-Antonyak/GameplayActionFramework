// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "ActiveGameplayActionEffectsContainer.h"
#include "GameFramework/Actor.h"

FActiveGameplayActionEffect* FActiveGameplayActionEffectsContainer::Find(const FActiveGameplayActionEffectHandle& Handle)
{
	return Effects.Find(Handle);
}

const FActiveGameplayActionEffect* FActiveGameplayActionEffectsContainer::Find(const FActiveGameplayActionEffectHandle& Handle) const
{
	return Effects.Find(Handle);
}

FActiveGameplayActionEffectHandle FActiveGameplayActionEffectsContainer::FindStackable(const UGameplayActionEffect* EffectCDO, const AActor* SourceActor) const
{
	if (!EffectCDO || EffectCDO->StackingPolicy == EGameplayActionEffectStacking::None)
	{
		return FActiveGameplayActionEffectHandle();
	}

	for (const TTuple<FActiveGameplayActionEffectHandle, FActiveGameplayActionEffect>& Pair : Effects)
	{
		const FActiveGameplayActionEffect& ExistingEffect = Pair.Value;

		if (ExistingEffect.Spec.Effect != EffectCDO)
		{
			continue;
		}

		if (EffectCDO->StackingPolicy == EGameplayActionEffectStacking::AggregateBySource)
		{
			if (ExistingEffect.Spec.SourceActor.Get() != SourceActor)
			{
				continue;
			}
		}
		
		return Pair.Key;
	}

	return FActiveGameplayActionEffectHandle();
}

FActiveGameplayActionEffectHandle FActiveGameplayActionEffectsContainer::Add(const FActiveGameplayActionEffect& NewEffect)
{
	if (!NewEffect.Handle.IsValid())
	{
		return FActiveGameplayActionEffectHandle();
	}

	Effects.Add(NewEffect.Handle, NewEffect);
	
	return NewEffect.Handle;
}

bool FActiveGameplayActionEffectsContainer::Remove(const FActiveGameplayActionEffectHandle& Handle)
{
	return Effects.Remove(Handle) > 0;
}
