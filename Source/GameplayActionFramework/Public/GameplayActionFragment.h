// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayActionFragment.generated.h"

/**
 * Gameplay Action Fragment is object with optional custom data for Data oriented approach
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionFragment : public UObject
{
	GENERATED_BODY()
};
