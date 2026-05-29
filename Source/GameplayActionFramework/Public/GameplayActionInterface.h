// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayActionInterface.generated.h"

class UGameplayActionComponent;

UINTERFACE()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that have a Gameplay Action Component.
 */
class GAMEPLAYACTIONFRAMEWORK_API IGameplayActionInterface
{
	GENERATED_BODY()

public:

	/** Returns the Gameplay Action Component of the actor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gameplay Action")
	UGameplayActionComponent* GetGameplayActionComponent() const;
};

