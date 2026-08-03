// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_MoveConstant.h"

#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"

UGameplayActionTask_MoveConstant::UGameplayActionTask_MoveConstant()
{
	bTickingTask = true;
}

UGameplayActionTask_MoveConstant* UGameplayActionTask_MoveConstant::MoveConstant
(
	UObject* WorldContextObject,
	FVector Direction, 
	float Speed, 
	float Duration, 
	UCurveFloat* SpeedOverTime
)
{
	UGameplayActionTask_MoveConstant* NewTask = NewGameplayActionTask<UGameplayActionTask_MoveConstant>(WorldContextObject);

	if (NewTask)
	{
		NewTask->MoveDirection = Direction;
		NewTask->MoveSpeed = Speed;
		NewTask->MoveDuration = Duration;
		NewTask->CurveFloat = SpeedOverTime;
		NewTask->ReadyForActivation();
	}
	
	return NewTask;
}

void UGameplayActionTask_MoveConstant::Activate()
{
	UGameplayActionComponent* Comp = GetActionComponent();
	
	if (!IsValid(Comp))
	{
		EndTask();
		
		return;
	}

	Super::Activate();
}

void UGameplayActionTask_MoveConstant::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	AActor* OwnerActor = GetAvatarActor();

	if (!IsValid(OwnerActor))
	{
		EndTask();
		
		return;
	}
	
	ElapsedTime += DeltaTime;
	
	const float SpeedMultiplier = CurveFloat ? CurveFloat->GetFloatValue(ElapsedTime) : 1.0f;
	
	const FVector DeltaMove = MoveDirection * MoveSpeed * SpeedMultiplier * DeltaTime;
	
	const FVector NewLocation = OwnerActor->GetActorLocation() + DeltaMove;
	
	const bool bIsMoved = OwnerActor->SetActorLocation(NewLocation, true);
	if (!bIsMoved)
	{
		EndTask();
		
		return;
	}

	if (ElapsedTime >= MoveDuration)
	{
		bMovementCompleted = true;
		
		OnMovementFinished.Broadcast();
		
		EndTask();
	}
}

void UGameplayActionTask_MoveConstant::OnDestroy(bool bInOwnerFinished)
{
	if (!bMovementCompleted)
	{
		OnMovementInterrupted.Broadcast();
	}
	
	Super::OnDestroy(bInOwnerFinished);
}
