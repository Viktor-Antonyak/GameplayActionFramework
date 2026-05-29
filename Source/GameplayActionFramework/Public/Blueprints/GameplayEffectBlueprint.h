// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameplayEffectBlueprint.generated.h"

UCLASS(BlueprintType)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayEffectBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:
	UGameplayEffectBlueprint(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
#endif
};
