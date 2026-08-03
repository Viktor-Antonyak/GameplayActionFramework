// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask_MoveTask.h"
#include "GameplayActionTask_MoveConstant.generated.h"

/**
 * Moves an Actor at a constant velocity in a given direction for a fixed duration.
 * 
 * @note Best for Actor, if you use this task with Character,
 * use RootMoveConstant instead, otherwise you may encounter some issues
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_MoveConstant : public UGameplayActionTask_MoveTask
{
	GENERATED_BODY()
	
public:
	
	UGameplayActionTask_MoveConstant();
	
	/** 
	 * Moves the Actor at a constant velocity in the given direction for the specified duration.
	 * @param Direction Movement direction (does not need to be normalized); scaled by Speed.
	 * @param Speed Movement speed in units per second.
	 * @param Duration Total time the movement lasts.
	 * @param SpeedOverTime Optional curve scaling speed over elapsed time (1.0 if none).
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_MoveConstant* MoveConstant
	(
		UObject* WorldContextObject,
		FVector Direction,
		float Speed,
		float Duration,
		UCurveFloat* SpeedOverTime
	);
	
protected:
	virtual void Activate() override;

	virtual void TickTask(float DeltaTime) override;

	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	FVector MoveDirection;
	
	float MoveSpeed = 0.0f;
	
	float ElapsedTime = 0.0f;
};
