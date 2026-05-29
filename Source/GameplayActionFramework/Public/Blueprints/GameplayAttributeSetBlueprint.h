// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameplayAttributeSetBlueprint.generated.h"

/**
 * A specialized Blueprint class for Gameplay Attribute Sets.
 */
UCLASS(BlueprintType)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayAttributeSetBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:
	UGameplayAttributeSetBlueprint(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	// UBlueprint interface
	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
	// End of UBlueprint interface
#endif
};
