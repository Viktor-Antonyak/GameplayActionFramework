// Copyright © 2026 Cyber Wood Team. All rights reserved.


#include "GameplayActionTasks/GameplayActionTask_WaitGameplayTagRemoved.h"
#include "GameplayActionComponent.h"

UGameplayActionTask_WaitGameplayTagRemoved::UGameplayActionTask_WaitGameplayTagRemoved()
{
	bTickingTask = false;
}

UGameplayActionTask_WaitGameplayTagRemoved* UGameplayActionTask_WaitGameplayTagRemoved::WaitGameplayTagRemoved
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
	
	UGameplayActionTask_WaitGameplayTagRemoved* NewTask = NewGameplayActionTask<UGameplayActionTask_WaitGameplayTagRemoved>(WorldContextObject);

	if (NewTask)
	{
		NewTask->TagToCheck = Tag;
		NewTask->bTriggerOnce = bOnlyTriggerOnce;
		NewTask->bExact = bExactTag;
	}
	
	return NewTask;
}

void UGameplayActionTask_WaitGameplayTagRemoved::Activate()
{
	if (!IsValid(GetActionComponent()))
	{
		EndTask();
		
		return;
	}
	
	Super::Activate();
	
	GetActionComponent()->OnGameplayTagsRemoved.AddDynamic(this, &UGameplayActionTask_WaitGameplayTagRemoved::HandleTag);
}

void UGameplayActionTask_WaitGameplayTagRemoved::OnDestroy(bool bInOwnerFinished)
{
	if (IsValid(GetActionComponent()))
	{
		GetActionComponent()->OnGameplayTagsRemoved.RemoveDynamic(this, &UGameplayActionTask_WaitGameplayTagRemoved::HandleTag);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UGameplayActionTask_WaitGameplayTagRemoved::HandleTag(const FGameplayTagContainer& Container)
{
	for (FGameplayTag Tag : Container)
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
			OnTagRemoved.Broadcast(Tag);
			OnTagRemovedNative.Broadcast(Tag);
		}
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}
