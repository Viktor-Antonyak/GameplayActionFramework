// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_RootMoveConstant.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UGameplayActionTask_RootMoveConstant::UGameplayActionTask_RootMoveConstant()
{
	bTickingTask = false;
}

UGameplayActionTask_RootMoveConstant* UGameplayActionTask_RootMoveConstant::RootMoveConstant
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
)
{
	UGameplayActionTask_RootMoveConstant* NewTask = NewGameplayActionTask<UGameplayActionTask_RootMoveConstant>(WorldContextObject);

	if (NewTask)
	{
		NewTask->RootMotionInstanceName = RootTaskName;
		NewTask->MoveDirection = Direction;
		NewTask->MoveSpeed = Speed;
		NewTask->MoveDuration = Duration;
		NewTask->CurveFloat = SpeedOverTime;
		NewTask->RootMotionAccumulateMode = RootMotionMode;
		NewTask->RootMotionFinishMode = RootMotionFinishVelocityMode;
		NewTask->bAllowGravity = bEnableGravity;
		NewTask->FinishClampVelocity = FinishVelocityClamp;
		NewTask->FinishSetVelocity = FinishVelocitySet;
	}
	
	return NewTask;
}

void UGameplayActionTask_RootMoveConstant::Activate()
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
	
	TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	
	ConstantForce->InstanceName = RootMotionInstanceName;
	ConstantForce->AccumulateMode = RootMotionAccumulateMode;
	ConstantForce->Priority = 1000;
	ConstantForce->Force = MoveDirection * MoveSpeed;
	ConstantForce->Duration = MoveDuration;
	ConstantForce->StrengthOverTime = CurveFloat;
	ConstantForce->FinishVelocityParams.Mode = RootMotionFinishMode;
	ConstantForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
	ConstantForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
	// Note: despite the flag's name, IgnoreZAccumulate actually ALLOWS gravity to affect
	if (bAllowGravity)
	{
		ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	}
	
	RootMotionSourceID = CachedCMC->ApplyRootMotionSource(ConstantForce);

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, 
			&UGameplayActionTask_RootMoveConstant::OnTimerFinished, MoveDuration, false);
	}
}

void UGameplayActionTask_RootMoveConstant::OnDestroy(bool bInOwnerFinished)
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

void UGameplayActionTask_RootMoveConstant::OnTimerFinished()
{
	bMovementCompleted = true;
	
	EndTask();
}
