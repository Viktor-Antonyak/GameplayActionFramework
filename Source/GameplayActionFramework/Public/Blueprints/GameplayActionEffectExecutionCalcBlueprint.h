// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameplayActionEffectExecutionCalcBlueprint.generated.h"

UCLASS(BlueprintType)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionEffectExecutionCalcBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
#endif
};
