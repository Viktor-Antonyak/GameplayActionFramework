// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask_MoveTask.h"
#include "GameFramework/RootMotionSource.h"
#include "GameplayActionTask_RootMoveTask.generated.h"

class UCharacterMovementComponent;

/**
 * Base class for Root Motion tasks
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_RootMoveTask : public UGameplayActionTask_MoveTask
{
	GENERATED_BODY()
	
protected:
	
	uint16 RootMotionSourceID = 0;
	
	FName RootMotionInstanceName = TEXT("RootMove");
	
	ERootMotionAccumulateMode RootMotionAccumulateMode = ERootMotionAccumulateMode::Override;
	
	ERootMotionFinishVelocityMode RootMotionFinishMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
	
	float FinishClampVelocity = 0.0f;
	
	FVector FinishSetVelocity;
	
	bool bAllowGravity = true;
	 
	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> CachedCMC = nullptr;
	 
	FTimerHandle TimerHandle;
};
