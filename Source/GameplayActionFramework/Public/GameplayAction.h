// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionActorInfo.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Templates/SharedPointer.h"
#include "Templates/SubclassOf.h"
#include "GameplayTaskOwnerInterface.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/ObjectMacros.h"

#include "GameplayAction.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayAction, Log, All);

class UGameplayActionComponent;
class UGameplayEffect;

/**
 * Type of the gameplay action.
 */
UENUM(BlueprintType)
enum class EGameplayActionType : uint8
{
	/** Action is added to the actor and can be triggered multiple times. Needs to be added via Add Gameplay Action. */
	Default,

	/** Action is spawned, executed once with custom data, and destroyed. Can't be added via Add Gameplay Action. */
	Triggered
};

/**
 * Action Cancellation Policy
 */
UENUM(BlueprintType)
enum class EGameplayActionCancelPolicy : uint8
{
    /** Activation fails if any targeted action cannot be cancelled */
    Block,

    /** Attempts to cancel but activates regardless of failures */
    IgnoreFailed
};

/**
 * Struct that defines the tags associated with a gameplay action.
 */
USTRUCT(BlueprintType)
struct FGameplayActionInfoTags
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Tags")
	FGameplayTag GameplayActionTag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Tags")
	FGameplayTagContainer BlockOtherActions;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Tags")
	FGameplayTagContainer CancelOtherActions;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Tags")
	FGameplayTagContainer RequireTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Tags")
	FGameplayTagContainer BlockedByTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Tags")
	FGameplayTagContainer GrantTags;
};

/**
 * Base class for gameplay actions.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayAction : public UObject, public IGameplayTaskOwnerInterface
{
	GENERATED_BODY()

public:
	UGameplayAction();

	/** Initializes the action with actor info and level. */
	virtual void InitializeAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel);

	/** Deinitializes the action. */
	virtual void DeinitializeAction();

	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	bool IsActive() const 
	{
		return bIsActive;
	}

	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	bool CanBeCanceled() const 
	{
		return bCanBeCanceled;
	}

	/** Used for Default actions. Primary used in Gameplay Action Component. */
	bool RequestExecuteAction();

	/** Used for Triggered actions. Primary used in Gameplay Action Component. */
	bool RequestTriggerAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel = -1, const FInstancedStruct& Payload = FInstancedStruct());

	/** Used to end the action. Primary used in Gameplay Action Component. */
	void RequestEndAction();

	/** Used to cancel the action. Primary used in Gameplay Action Component. */
	bool RequestCancelAction();

	FGameplayTag GetActionTag() const 
	{
		return GameplayActionInfoTags.GameplayActionTag;
	}
	FGameplayTagContainer GetBlockOtherActionsTags() const 
	{
		return GameplayActionInfoTags.BlockOtherActions;
	}
	FGameplayTagContainer GetCancelOtherActionsTags() const 
	{
		return GameplayActionInfoTags.CancelOtherActions;
	}

protected:

	virtual UWorld* GetWorld() const override;

	// IGameplayTaskOwnerInterface
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;

	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void EndAction();

	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void CancelAction();

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Cost")
	bool CommitCost();

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Cooldown")
	bool CommitCooldown();

	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void SetCanBeCanceled(bool CanBeCanceled) 
	{
		bCanBeCanceled = CanBeCanceled;
	}
	
	/** Called when the action is executed (Standard actions). */
	UFUNCTION(BlueprintNativeEvent, Category="Gameplay Action")
	void OnExecuteAction();

	/** Event called when action is triggered with optional payload (Triggered actions). */
	UFUNCTION(BlueprintNativeEvent, Category="Gameplay Action")
	void OnActionTriggered(const FInstancedStruct& Payload);

	/** Called when the action ends. */
	UFUNCTION(BlueprintNativeEvent, Category="Gameplay Action")
	void OnEndAction(bool bWasCanceled = false);

	/** Overridable function to check if the action can be executed. */
	UFUNCTION(BlueprintNativeEvent, Category="Gameplay Action")
	bool CanExecuteAction() const;

	UFUNCTION(BlueprintCallable, Category="Gameplay Action | Action Info")
	int32 GetActionLevel() const 
	{
		return CachedActionLevel;
	}

	UFUNCTION(BlueprintCallable, Category="Gameplay Action | Action Info")
	AActor* GetOwnerActor() const 
	{
		return CachedActorInfo ? CachedActorInfo->OwnerActor.Get() : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category="Gameplay Action | Action Info")
	TArray<USkeletalMeshComponent*> GetSkeletalMeshComponents() const;

	UFUNCTION(BlueprintCallable, Category="Gameplay Action | Action Info")
	UMovementComponent* GetMovementComponent() const 
	{ 
		return CachedActorInfo ? CachedActorInfo->MovementComponent.Get() : nullptr; 
	}

	UFUNCTION(BlueprintCallable, Category="Gameplay Action | Action Info")
	UGameplayActionComponent* GetActionComponent() const 
	{ 
		return CachedActorInfo ? CachedActorInfo->ActionComponent.Get() : nullptr; 
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay Action Info", meta=(ShowOnlyInnerProperties))
	FGameplayActionInfoTags GameplayActionInfoTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Info")
	EGameplayActionCancelPolicy CancelPolicy = EGameplayActionCancelPolicy::Block;

	/** Type of this action. Standard actions are added via AddGameplayAction. Triggered are executed on the fly. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Action Type")
	EGameplayActionType ActionType = EGameplayActionType::Default;

	/** Input ID associated with the action (Standard actions only). */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input", meta=(EditCondition="ActionType == EGameplayActionType::Default"))
	int32 InputID;

	/** Cooldown effect to apply when the action is executed. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffect;

	/** Cost of the action. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Cost")
	TSubclassOf<UGameplayEffect> CostEffect;

private:

	bool bIsActive = false;

	TSharedPtr<FGameplayActionActorInfo> CachedActorInfo;

	int32 CachedActionLevel = -1;

	bool bCanBeCanceled = true;

	bool CanCancelAllActions();

	bool CheckBlockedTags();

	bool CheckRequiredTags();

	bool CheckBlockedActionTags();

	bool CanApplyCost();

	UPROPERTY()
	TArray<USkeletalMeshComponent*> MeshComponents;

	friend class UGameplayActionComponent;
	friend class UGameplayActionFactory;
};
