// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "GameplayActionInterface.h"
#include "GameplayAction.h"
#include "GameplayActionComponent.h"
#include "GameplayActionTask.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayActionTask, Log, All);

/**
 * Base class for Gameplay Action Task
 */
UCLASS(Abstract, BlueprintType)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask : public UGameplayTask
{
	GENERATED_BODY()
	
public:
	
	template<class TaskType>
	static TaskType* NewGameplayActionTask(UObject* WorldContextObject)
	{
		if (!IsValid(WorldContextObject))
		{
			return nullptr;
		}

		if (!WorldContextObject->Implements<UGameplayActionInterface>())
		{
			return nullptr;
		}
		
		UGameplayActionComponent* OwnerComponent = IGameplayActionInterface::Execute_GetGameplayActionComponent(WorldContextObject);
		
		IGameplayTaskOwnerInterface* OwnerInterface;

		if (UGameplayAction* Action = Cast<UGameplayAction>(WorldContextObject))
		{
			OwnerInterface = Action;
		}
		else
		{
			OwnerInterface = OwnerComponent;
		}

		if (!OwnerComponent || !OwnerInterface)
		{
			UE_LOG(LogGameplayActionTask, Log, TEXT("Could not resolve a valid task owner from %s"), *WorldContextObject->GetName())
			
			return nullptr;
		}
		
		TaskType* NewTask = NewObject<TaskType>();
		NewTask->InitTask(*OwnerInterface, NewTask->GetPriority());
		NewTask->CachedGameplayActionComponent = OwnerComponent;
		NewTask->CachedGameplayAction = Cast<UGameplayAction>(WorldContextObject);
		return NewTask;
	}
	
protected:
	
	UGameplayActionComponent* GetActionComponent() const
	{
		return CachedGameplayActionComponent.Get();
	}
	
	UPROPERTY()
	TWeakObjectPtr<UGameplayActionComponent> CachedGameplayActionComponent;
	
	UPROPERTY()
	TWeakObjectPtr<UGameplayAction> CachedGameplayAction;
};
