// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_MoveToLocation.h"

#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"

UGameplayActionTask_MoveToLocation::UGameplayActionTask_MoveToLocation()
{
	bTickingTask = true;
}

UGameplayActionTask_MoveToLocation* UGameplayActionTask_MoveToLocation::MoveToLocation
(
	UObject* WorldContextObject, 
	FVector TargetLocation, 
	float Duration,
	UCurveFloat* EasingCurve
)
{
	UGameplayActionTask_MoveToLocation* NewTask = NewGameplayActionTask<UGameplayActionTask_MoveToLocation>(WorldContextObject);
	
	if (NewTask)
	{
		NewTask->EndLocation = TargetLocation;
		NewTask->MoveDuration = Duration;
		NewTask->CurveFloat = EasingCurve;
		NewTask->ReadyForActivation();
	}
	
	return NewTask;
}

void UGameplayActionTask_MoveToLocation::Activate()
{
	AActor* OwnerActor = GetAvatarActor();
	
	if (!IsValid(OwnerActor))
	{
		EndTask();
		
		return;
	}
	
	StartLocation = OwnerActor->GetActorLocation();
		
	Super::Activate();
}

void UGameplayActionTask_MoveToLocation::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	AActor* OwnerActor = GetAvatarActor();

	if (!OwnerActor)
	{
		EndTask();
		
		return;
	}
	
	ElapsedTime +=DeltaTime;
	
	float Alpha = FMath::Clamp(ElapsedTime / MoveDuration, 0.f, 1.0f);
	
	float InterpolationAlpha = Alpha;

	if (CurveFloat)
	{
		InterpolationAlpha = CurveFloat->GetFloatValue(Alpha);
	}
	
	FVector CurrentPos = FMath::Lerp(StartLocation, EndLocation, InterpolationAlpha);
	
	bool bIsMoved = OwnerActor->SetActorLocation(CurrentPos, true);

	if (!bIsMoved)
	{
		EndTask();
		
		return;
	}
	
	if (Alpha >= 1.0f)
	{
		bMovementCompleted = true;
		
		OnMovementFinished.Broadcast();
		
		EndTask();
	}
}

void UGameplayActionTask_MoveToLocation::OnDestroy(bool bInOwnerFinished)
{
	if (!bMovementCompleted)
	{
		OnMovementInterrupted.Broadcast();
	}
	
	Super::OnDestroy(bInOwnerFinished);
}
