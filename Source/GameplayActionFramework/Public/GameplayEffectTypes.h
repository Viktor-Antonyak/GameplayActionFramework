// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.generated.h"

/** Operations for Attributes */
UENUM(BlueprintType)
enum class EGameplayModOp : uint8
{
	Additive,
	Multiplicative,
	Override
};

USTRUCT(BlueprintType)
struct FActiveGameplayEffectHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Handle = -1;

	bool IsValid() const
	{
		return Handle > 0;
	}

	bool operator==(const FActiveGameplayEffectHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FActiveGameplayEffectHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	friend uint32 GetTypeHash(const FActiveGameplayEffectHandle& InHandle)
	{
		return GetTypeHash(InHandle.Handle);
	}
};