// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask.h"
#include "GameplayActionTasks/GameplayActionTask_MoveTask.h"
#include "GameplayActionTask_MoveToLocation.generated.h"


/**
 * Move Actor to specific location over time
 * 
 * @note Best for Actor, if you use this task with Character,
 * use RootMoveToLocation instead, otherwise you may encounter some issues
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_MoveToLocation : public UGameplayActionTask_MoveTask
{
	GENERATED_BODY()
	
public:
	
	UGameplayActionTask_MoveToLocation();
	
	/** 
	 * Moves the Actor to the given world location over the specified duration.
	 * @param TargetLocation Destination world position.
	 * @param Duration Time the movement takes.
	 * @param EasingCurve Optional curve mapping interpolation alpha (0-1) to ease the movement.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks", 
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_MoveToLocation* MoveToLocation
	(
		UObject* WorldContextObject, 
		FVector TargetLocation, 
		float Duration,
		UCurveFloat* EasingCurve
	);

protected:
	
	virtual void Activate() override;	
	
	virtual void TickTask(float DeltaTime) override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	FVector StartLocation;
	
	FVector EndLocation;
	
	float ElapsedTime = 0.0f;
};
