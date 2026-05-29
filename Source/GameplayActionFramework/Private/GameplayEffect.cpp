// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayEffect.h"

void FGameplayEffectSpec::SetSetByCallerMagnitude(FGameplayTag Tag, float Magnitude)
{
    if (Tag.IsValid())
    {
        SetByCallerMagnitudes.FindOrAdd(Tag) = Magnitude;
    }
}

float FGameplayEffectSpec::GetSetByCallerMagnitude(FGameplayTag Tag) const
{
    return SetByCallerMagnitudes.FindRef(Tag);
}
