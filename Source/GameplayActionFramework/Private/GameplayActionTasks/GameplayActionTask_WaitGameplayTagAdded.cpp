// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_WaitGameplayTagAdded.h"
#include "GameplayActionComponent.h"

UGameplayActionTask_WaitGameplayTagAdded::UGameplayActionTask_WaitGameplayTagAdded()
{
	bTickingTask = false;
}

UGameplayActionTask_WaitGameplayTagAdded* UGameplayActionTask_WaitGameplayTagAdded::WaitGameplayTagAdded
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
	
	UGameplayActionTask_WaitGameplayTagAdded* NewTask = NewGameplayActionTask<UGameplayActionTask_WaitGameplayTagAdded>(WorldContextObject);

	if (NewTask)
	{
		NewTask->TagToCheck = Tag;
		NewTask->bTriggerOnce = bOnlyTriggerOnce;
		NewTask->bExact = bExactTag;
	}
	
	return NewTask;
}

void UGameplayActionTask_WaitGameplayTagAdded::Activate()
{
	UGameplayActionComponent* Component = GetActionComponent();
	
	if (!IsValid(Component))
	{
		EndTask();
		
		return;
	}
	
	Super::Activate();
	
	Component->OnGameplayTagsAdded.AddDynamic(this, &UGameplayActionTask_WaitGameplayTagAdded::HandleTag);
}

void UGameplayActionTask_WaitGameplayTagAdded::OnDestroy(bool bInOwnerFinished)
{
	UGameplayActionComponent* Component = GetActionComponent();
	
	if (IsValid(Component))
	{
		Component->OnGameplayTagsAdded.RemoveDynamic(this, &UGameplayActionTask_WaitGameplayTagAdded::HandleTag);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UGameplayActionTask_WaitGameplayTagAdded::HandleTag(const FGameplayTagContainer& Container)
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
			OnTagAdded.Broadcast(Tag);
			OnTagAddedNative.Broadcast(Tag);
		}
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}
