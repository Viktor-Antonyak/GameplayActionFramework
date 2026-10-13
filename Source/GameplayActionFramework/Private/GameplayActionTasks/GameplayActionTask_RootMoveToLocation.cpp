// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_RootMoveToLocation.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UGameplayActionTask_RootMoveToLocation::UGameplayActionTask_RootMoveToLocation()
{
	bTickingTask = false;
}

UGameplayActionTask_RootMoveToLocation* UGameplayActionTask_RootMoveToLocation::RootMoveToLocation
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
)
{
	UGameplayActionTask_RootMoveToLocation* NewTask = NewGameplayActionTask<UGameplayActionTask_RootMoveToLocation>(WorldContextObject);

	if (NewTask)
	{
		NewTask->RootMotionInstanceName = RootTaskName;
		NewTask->GoalLocation = TargetLocation;
		NewTask->MoveDuration = Duration;
		NewTask->CurveVector = PathOffsetCurve;
		NewTask->RootMotionAccumulateMode = RootMotionMode;
		NewTask->RootMotionFinishMode = RootMotionFinishVelocityMode;
		NewTask->bAllowGravity = bEnableGravity;
		NewTask->FinishClampVelocity = FinishVelocityClamp;
		NewTask->FinishSetVelocity = FinishVelocitySet;
	}
	
	return NewTask;
}

void UGameplayActionTask_RootMoveToLocation::Activate()
{
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	
	if (!Character)
	{
		EndTask();
		
		return;
	}
	
	CachedCMC = Character->GetCharacterMovement();

	if (!CachedCMC.IsValid())
	{
		EndTask();
		
		return;
	}
	
	Super::Activate();
	
	TSharedPtr<FRootMotionSource_MoveToForce> MoveToForce = MakeShared<FRootMotionSource_MoveToForce>();
	
	MoveToForce->InstanceName = RootMotionInstanceName;
	MoveToForce->AccumulateMode = RootMotionAccumulateMode;
	MoveToForce->Priority = 1000;
	MoveToForce->StartLocation = Character->GetActorLocation();
	MoveToForce->TargetLocation = GoalLocation;
	MoveToForce->Duration = MoveDuration;
	MoveToForce->PathOffsetCurve = CurveVector;
	MoveToForce->FinishVelocityParams.Mode = RootMotionFinishMode;
	MoveToForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
	MoveToForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
	// Note: despite the flag's name, IgnoreZAccumulate actually ALLOWS gravity to affect
	if (bAllowGravity)
	{
		MoveToForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	}
	
	RootMotionSourceID = CachedCMC->ApplyRootMotionSource(MoveToForce);

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, 
			&UGameplayActionTask_RootMoveToLocation::OnTimerFinished, MoveDuration, false);
	}
}

void UGameplayActionTask_RootMoveToLocation::OnDestroy(bool bInOwnerFinished)
{
	if (CachedCMC.IsValid() && RootMotionSourceID > 0)
	{
		CachedCMC->RemoveRootMotionSourceByID(RootMotionSourceID);
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	
	if (bMovementCompleted)
	{
		OnMovementFinished.Broadcast();
		OnMovementFinishedNative.Broadcast();
	}
	else
	{
		OnMovementInterrupted.Broadcast();
		OnMovementInterruptedNative.Broadcast();
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UGameplayActionTask_RootMoveToLocation::OnTimerFinished()
{
	bMovementCompleted = true;
	
	EndTask();
}
