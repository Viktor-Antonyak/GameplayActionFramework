// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameplayActionBlueprint.generated.h"

/**
 * A Gameplay Action Blueprint is a specialized Blueprint whose graphs control a gameplay action.
 */
UCLASS(BlueprintType)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:
	UGameplayActionBlueprint(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	// UBlueprint interface
	virtual bool SupportedByDefaultBlueprintFactory() const override
	{
		return false;
	}
	// End of UBlueprint interface
#endif
};
