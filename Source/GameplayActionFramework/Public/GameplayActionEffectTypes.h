// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionEffectTypes.generated.h"

/** Operations for Attributes */
UENUM(BlueprintType)
enum class EGameplayActionEffectModOp : uint8
{
	Additive,
	Multiplicative,
	Override
};

USTRUCT(BlueprintType)
struct FActiveGameplayActionEffectHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Handle = -1;

	bool IsValid() const
	{
		return Handle > 0;
	}

	bool operator==(const FActiveGameplayActionEffectHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FActiveGameplayActionEffectHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	friend uint32 GetTypeHash(const FActiveGameplayActionEffectHandle& InHandle)
	{
		return GetTypeHash(InHandle.Handle);
	}
};