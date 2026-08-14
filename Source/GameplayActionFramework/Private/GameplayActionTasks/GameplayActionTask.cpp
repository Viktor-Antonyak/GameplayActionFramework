// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask.h"
#include "GameplayActionInterface.h"
#include "GameplayAction.h"
#include "GameplayActionComponent.h"

DEFINE_LOG_CATEGORY(LogGameplayActionTask)

UGameplayActionTask* UGameplayActionTask::NewTaskInternal(UClass* TaskClass, UObject* WorldContextObject)
{
	if (!IsValid(TaskClass) || !IsValid(WorldContextObject))
	{
		UE_LOG(LogGameplayActionTask, Warning, TEXT("NewGameplayActionTask failed: Invalid TaskClass or WorldContextObject."));
		return nullptr;
	}

	if (!WorldContextObject->Implements<UGameplayActionInterface>())
	{
		UE_LOG(LogGameplayActionTask, Warning, TEXT("NewGameplayActionTask failed: WorldContextObject '%s' does not implement UGameplayActionInterface."), *WorldContextObject->GetName());
		return nullptr;
	}

	UGameplayActionComponent* OwnerComponent = IGameplayActionInterface::Execute_GetGameplayActionComponent(WorldContextObject);
	if (!IsValid(OwnerComponent))
	{
		UE_LOG(LogGameplayActionTask, Error, TEXT("NewGameplayActionTask failed: Could not resolve valid GameplayActionComponent from '%s'."), *WorldContextObject->GetName());
		return nullptr;
	}

	IGameplayTaskOwnerInterface* TaskOwner = nullptr;

	if (UGameplayAction* Action = Cast<UGameplayAction>(WorldContextObject))
	{
		TaskOwner = Action;
	}
	else
	{
		TaskOwner = OwnerComponent;
	}

	if (!TaskOwner)
	{
		UE_LOG(LogGameplayActionTask, Error, TEXT("NewGameplayActionTask failed: Could not resolve valid TaskOwner interface from '%s'."), *WorldContextObject->GetName());
		return nullptr;
	}
	
	UGameplayActionTask* NewTask = NewObject<UGameplayActionTask>(WorldContextObject, TaskClass);
	
	if (!NewTask)
	{
		UE_LOG(LogGameplayActionTask, Error, TEXT("NewGameplayActionTask failed: Instantiation failed for task class '%s'."), *TaskClass->GetName());
		return nullptr;
	}

	NewTask->InitTask(*TaskOwner, NewTask->GetPriority());
	NewTask->CachedGameplayActionComponent = OwnerComponent;
	NewTask->CachedGameplayAction = Cast<UGameplayAction>(WorldContextObject);

	return NewTask;
}
