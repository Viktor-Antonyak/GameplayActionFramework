// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAction.h"
#include "GameplayActionSpec.generated.h"

/** Spec for a gameplay action added to the component. */
USTRUCT(BlueprintType)
struct FGameplayActionSpec
{
	GENERATED_BODY()
	
public:
	
	/** The gameplay action instance. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Spec")
	UGameplayAction* Action = nullptr;
	
	/** Action power level. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Spec")
	int32 Level = -1;
	
	/** Input binding ID (copied from the action class). */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Action Spec")
	int32 InputID = -1;

	/** Returns whether the action pointer is valid. */
	bool IsValid() const
	{
		return Action != nullptr;
	}
	
	bool operator==(const FGameplayActionSpec& Other) const
	{
		return Action == Other.Action && Level == Other.Level && InputID == Other.InputID;
	}
	
	bool operator!=(const FGameplayActionSpec& Other) const
	{
		return !operator==(Other);
	}
};
