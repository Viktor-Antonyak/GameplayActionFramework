// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayActionTask_WaitAttributeChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChangedDelegate, FGameplayActionAttribute, Attribute, float, NewValue, float, OldValue);

/**
 * Waits for a specific Gameplay Attribute's numeric value to change on the owning GameplayActionComponent.
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_WaitAttributeChanged : public UGameplayActionTask
{
	GENERATED_BODY()
	
public:
	
	/** 
	 * @param Attribute Attribute to watch for changes.
	 * @param bOnlyTriggerOnce If true, the task ends after the first change. If false, it keeps firing on every change.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_WaitAttributeChanged* WaitAttributeChanged
	(
		UObject* WorldContextObject,
		FGameplayActionAttribute Attribute, 
		bool bOnlyTriggerOnce = false
	);
	
	/** Fired when the watched attribute's value changes. */
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedDelegate OnAttributeChanged;
	
protected:
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UFUNCTION()
	void HandleAttribute(const FGameplayActionAttribute& Attribute, float NewValue, float OldValue);
	
	FGameplayActionAttribute WatchedAttribute;
	
	bool bTriggerOnce = false;
};
