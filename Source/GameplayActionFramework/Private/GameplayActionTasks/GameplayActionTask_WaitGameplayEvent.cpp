// Copyright © 2026 Cyber Wood Team. All rights reserved.


#include "GameplayActionTasks/GameplayActionTask_WaitGameplayEvent.h"

#include "GameplayActionComponent.h"

UGameplayActionTask_WaitGameplayEvent::UGameplayActionTask_WaitGameplayEvent()
{
	bTickingTask = false;
}

UGameplayActionTask_WaitGameplayEvent* UGameplayActionTask_WaitGameplayEvent::WaitGameplayEvent
(
	UObject* WorldContextObject, 
	FGameplayTag Tag, 
	bool bExactTag, 
	bool bOnlyTriggerOnce
)
{
	if (!Tag.IsValid())
	{
		return nullptr;
	}
	
	UGameplayActionTask_WaitGameplayEvent* NewTask = NewGameplayActionTask<UGameplayActionTask_WaitGameplayEvent>(WorldContextObject);

	if (NewTask)
	{
		NewTask->TagToCheck = Tag;
		NewTask->bTriggerOnce = bOnlyTriggerOnce;
		NewTask->bExact = bExactTag;
	}
	
	return NewTask;
}

void UGameplayActionTask_WaitGameplayEvent::Activate()
{
	if (!IsValid(GetActionComponent()))
	{
		EndTask();
		
		return;
	}
	
	Super::Activate();
	
	GetActionComponent()->OnGameplayActionEvent.AddDynamic(this, &UGameplayActionTask_WaitGameplayEvent::HandleEventReceived);
}

void UGameplayActionTask_WaitGameplayEvent::OnDestroy(bool bInOwnerFinished)
{
	if (IsValid(GetActionComponent()))
	{
		GetActionComponent()->OnGameplayActionEvent.RemoveDynamic(this, &UGameplayActionTask_WaitGameplayEvent::HandleEventReceived);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UGameplayActionTask_WaitGameplayEvent::HandleEventReceived(const FGameplayTag& Tag)
{
	bool bMatch = false;
		
	if (bExact)
	{
		bMatch = Tag.MatchesTagExact(TagToCheck);
	}
	else
	{
		bMatch = Tag.MatchesTag(TagToCheck);
	}

	if (bMatch)
	{
		OnGameplayEvent.Broadcast(Tag);
		OnGameplayEventNative.Broadcast(Tag);
	}
}
