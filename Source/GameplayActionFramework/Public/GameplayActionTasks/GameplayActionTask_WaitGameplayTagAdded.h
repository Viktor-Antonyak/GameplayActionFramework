// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask.h"
#include "GameplayTagContainer.h"
#include "GameplayActionTask_WaitGameplayTagAdded.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagAddedTaskDelegate, FGameplayTag, ActualTag);

/**
 * Waits until a specific Gameplay Tag is added on the owning GameplayActionComponent.
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_WaitGameplayTagAdded : public UGameplayActionTask
{
	GENERATED_BODY()
	
public:
	
	/** 
	 * Waits for a Gameplay Tag to be added to the owning GameplayActionComponent.
	 * @param Tag The tag to listen for.
	 * @param bExactTag If true, only the exact tag matches; otherwise parent tags match their children too.
	 * @param bOnlyTriggerOnce If true, the task ends after the first tag-added broadcast that contains a matching tag. If false, the task keeps firing on every add.
	 * @note The delegate fires once per matching tag inside that container broadcast, so a broadcast containing several matching tags fires it multiple times before the task ends.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_WaitGameplayTagAdded* WaitGameplayTagAdded
	(
		UObject* WorldContextObject,
		FGameplayTag Tag,
		bool bExactTag,
		bool bOnlyTriggerOnce = false
	);
	
	/** Fired when a matching tag is added; broadcasts the actual tag that matched. Fires once per matching tag in the added tags container. */
	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedTaskDelegate OnTagAdded;
	
protected:
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UFUNCTION()
	void HandleTag(const FGameplayTagContainer& Container);
	
	FGameplayTag TagToCheck;
	
	bool bExact;
	
	bool bTriggerOnce = false;
};
