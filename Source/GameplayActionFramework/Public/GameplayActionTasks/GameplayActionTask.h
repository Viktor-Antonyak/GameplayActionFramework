// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "GameplayActionTask.generated.h"

class UGameplayAction;
class UGameplayActionComponent;

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
		return Cast<TaskType>(NewTaskInternal(TaskType::StaticClass(), WorldContextObject));
	}
	
protected:
	
	static UGameplayActionTask* NewTaskInternal(UClass* TaskClass, UObject* WorldContextObject);
	
	UGameplayActionComponent* GetActionComponent() const
	{
		return CachedGameplayActionComponent.Get();
	}
	
	UGameplayAction* GetGameplayAction() const
	{
		return CachedGameplayAction.Get();
	}
	
	TWeakObjectPtr<UGameplayActionComponent> CachedGameplayActionComponent;
	
	TWeakObjectPtr<UGameplayAction> CachedGameplayAction;
};
