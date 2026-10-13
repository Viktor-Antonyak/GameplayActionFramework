// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTasks/GameplayActionTask_RootMoveTask.h"
#include "GameFramework/RootMotionSource.h"
#include "GameplayActionTask_RootMoveToLocation.generated.h"


/**
 * Move Character to specific location over time
 * 
 * @note Best for Character, if you use this task with Actor,
 * use MoveToLocation instead, otherwise task will fail.
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_RootMoveToLocation : public UGameplayActionTask_RootMoveTask
{
	GENERATED_BODY()
	
public:
	
	UGameplayActionTask_RootMoveToLocation();
	
	/** 
	 * Moves the Character to the given world location over the specified duration using root motion.
	 * @param RootTaskName Name used to identify the root motion source instance.
	 * @param TargetLocation Destination world position.
	 * @param Duration Time the movement takes.
	 * @param PathOffsetCurve Optional curve that offsets the straight path to the target over time.
	 * @param RootMotionMode How this source combines with other active root motion sources.
	 * @param RootMotionFinishVelocityMode How velocity is handled when the source finishes.
	 * @param bEnableGravity If true, gravity keeps affecting the Character during the move.
	 * @param FinishVelocityClamp Velocity clamp applied when FinishVelocityMode is ClampVelocity.
	 * @param FinishVelocitySet Velocity applied when FinishVelocityMode is SetVelocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks", 
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_RootMoveToLocation* RootMoveToLocation
	(
		UObject* WorldContextObject, 
		FName RootTaskName, 
		FVector TargetLocation, 
		float Duration,
		UCurveVector* PathOffsetCurve,
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
	
	FVector GoalLocation;
	
	UPROPERTY()
	TObjectPtr<UCurveVector> CurveVector;
};
