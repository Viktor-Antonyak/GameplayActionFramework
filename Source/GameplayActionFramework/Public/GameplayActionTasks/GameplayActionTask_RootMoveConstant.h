// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask_RootMoveTask.h"
#include "GameplayActionTask_RootMoveConstant.generated.h"

/**
 * Applies constant velocity root motion to a Character for a fixed duration.
 *
 * @note Best for Character. If you use this task with Actor, use MoveConstant instead, otherwise task will fail.
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_RootMoveConstant : public UGameplayActionTask_RootMoveTask
{
	GENERATED_BODY()
	
public:
	
	UGameplayActionTask_RootMoveConstant();
	
	/** 
	 * Applies constant velocity root motion to the Character for the specified duration.
	 * @param RootTaskName Name used to identify the root motion source instance.
	 * @param Direction Movement direction (does not need to be normalized); scaled by Speed.
	 * @param Speed Movement speed in units per second.
	 * @param Duration Total time the root motion lasts.
	 * @param SpeedOverTime Optional curve scaling speed over elapsed time (1.0 if none).
	 * @param RootMotionMode How this source combines with other active root motion sources.
	 * @param RootMotionFinishVelocityMode How velocity is handled when the source finishes.
	 * @param bEnableGravity If true, gravity keeps affecting the Character during the move.
	 * @param FinishVelocityClamp Velocity clamp applied when FinishVelocityMode is ClampVelocity.
	 * @param FinishVelocitySet Velocity applied when FinishVelocityMode is SetVelocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_RootMoveConstant* RootMoveConstant
	(
		UObject* WorldContextObject,
		FName RootTaskName,
		FVector Direction,
		float Speed, 
		float Duration,
		UCurveFloat* SpeedOverTime,
		ERootMotionAccumulateMode RootMotionMode,
		ERootMotionFinishVelocityMode RootMotionFinishVelocityMode, 
		bool bEnableGravity,
		float FinishVelocityClamp, 
		FVector FinishVelocitySet
	);
	
protected:
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;

	void OnTimerFinished();

	FVector MoveDirection;
	
	float MoveSpeed = 0.0f;
};
