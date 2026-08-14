// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_WaitAttributeChanged.h"
#include "GameplayActionComponent.h"

UGameplayActionTask_WaitAttributeChanged* UGameplayActionTask_WaitAttributeChanged::WaitAttributeChanged
(
	UObject* WorldContextObject,
	FGameplayActionAttribute Attribute,
	bool bOnlyTriggerOnce
)
{
	UGameplayActionTask_WaitAttributeChanged* NewTask = NewGameplayActionTask<UGameplayActionTask_WaitAttributeChanged>(WorldContextObject);

	if (NewTask)
	{
		NewTask->WatchedAttribute = Attribute;
		NewTask->bTriggerOnce = bOnlyTriggerOnce;
		NewTask->ReadyForActivation();
	}
	
	return NewTask;
}

void UGameplayActionTask_WaitAttributeChanged::Activate()
{
	UGameplayActionComponent* Component = GetActionComponent();
	
	if (!IsValid(Component))
	{
		EndTask();
		
		return;
	}
	
	Super::Activate();
	
	Component->OnAttributeChanged.AddDynamic(this, &UGameplayActionTask_WaitAttributeChanged::HandleAttribute);
}

void UGameplayActionTask_WaitAttributeChanged::OnDestroy(bool bInOwnerFinished)
{
	UGameplayActionComponent* Component = GetActionComponent();
	
	if (IsValid(Component))
	{
		Component->OnAttributeChanged.RemoveDynamic(this, &UGameplayActionTask_WaitAttributeChanged::HandleAttribute);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UGameplayActionTask_WaitAttributeChanged::HandleAttribute(const FGameplayActionAttribute& Attribute, float NewValue, float OldValue)
{
	if (Attribute != WatchedAttribute)
	{
		return;
	}
	
	OnAttributeChanged.Broadcast(Attribute, NewValue, OldValue);

	if (bTriggerOnce)
	{
		EndTask();
	}
}
