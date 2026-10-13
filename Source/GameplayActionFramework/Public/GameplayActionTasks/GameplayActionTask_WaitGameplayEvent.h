// Copyright © 2026 Cyber Wood Team. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask.h"
#include "GameplayTagContainer.h"
#include "GameplayActionTask_WaitGameplayEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayEventDelegate, FGameplayTag, EventTag);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameplayEventNativeDelegate, FGameplayTag EventTag);

/**
 * Waits until a specific Gameplay Event is sent to GameplayActionComponent.
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_WaitGameplayEvent : public UGameplayActionTask
{
	GENERATED_BODY()
	
public:
	
	UGameplayActionTask_WaitGameplayEvent();
	
	/** 
	 * Wait for a Gameplay Event to be sent to GameplayActionComponent.
	 * @param Tag The tag to listen for.
	 * @param bExactTag If true, only the exact tag matches; otherwise parent tags match their children too.
	 * @param bOnlyTriggerOnce If true, the task ends after the first Gameplay Event broadcast that contains a matching tag. If false, the task keeps firing on every event.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_WaitGameplayEvent* WaitGameplayEvent
	(
		UObject* WorldContextObject, 
		FGameplayTag Tag, 
		bool bExactTag, 
		bool bOnlyTriggerOnce
	);
	
	/** Fired when a Gameplay Event is sent. */
	UPROPERTY(BlueprintAssignable)
	FOnGameplayEventDelegate OnGameplayEvent;
	
	FOnGameplayEventNativeDelegate OnGameplayEventNative;
	
protected:
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UFUNCTION()
	void HandleEventReceived(const FGameplayTag& Tag);

	FGameplayTag TagToCheck;
	
	bool bExact = true;
	
	bool bTriggerOnce = true;
};
