// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTasks/GameplayActionTask.h"
#include "GameplayActionTask_MoveTask.generated.h"

class UCurveFloat;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementFinished);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementInterrupted);


/**
 * Base class for movement tasks
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_MoveTask : public UGameplayActionTask
{
	GENERATED_BODY()
	
public:
	
	/** Fired when the task ends before reaching its target (interrupted or failed). */
	UPROPERTY(BlueprintAssignable)
	FOnMovementInterrupted OnMovementInterrupted;
	
	/** Fired when the movement completes successfully. */
	UPROPERTY(BlueprintAssignable)
	FOnMovementFinished OnMovementFinished;
	
protected:
	
	float MoveDuration = 0.0f;
	
	bool bMovementCompleted = false;
	
	UPROPERTY()
	TObjectPtr<UCurveFloat> CurveFloat = nullptr;
};
