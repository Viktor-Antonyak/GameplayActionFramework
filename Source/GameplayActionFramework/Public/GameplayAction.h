// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayActionInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTaskOwnerInterface.h"
#include "GameplayActionTypes.h"
#include "GameplayActionEffect.h"

// InstancedStruct.h path differs across engine versions (StructUtils plugin vs engine-native).
// __has_include avoids hardcoding a specific version boundary, which the maintainer cannot
// verify across all supported engine versions. If you hit a compile error here on your engine
// version, please open an issue with your exact UE version.
#if __has_include("StructUtils/InstancedStruct.h")
#include "StructUtils/InstancedStruct.h"
#else
#include "InstancedStruct.h"
#endif

#include "GameplayAction.generated.h"

GAMEPLAYACTIONFRAMEWORK_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplayAction, Log, All);

class UGameplayActionComponent;

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
    /** Activation fails if any targeted action cannot be canceled */
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

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Tags")
	FGameplayTag GameplayActionTag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Tags")
	FGameplayTagContainer BlockOtherActions;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Tags")
	FGameplayTagContainer CancelOtherActions;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Tags")
	FGameplayTagContainer RequireTags;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Tags")
	FGameplayTagContainer BlockedByTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Tags")
	FGameplayTagContainer GrantTags;
};

/**
 * Base class for gameplay actions.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayAction : public UObject, public IGameplayTaskOwnerInterface, public IGameplayActionInterface
{
	GENERATED_BODY()

public:
	
	UGameplayAction();

	/** Initializes the action with actor info and level. */
	virtual void InitializeAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel);

	/** Deinitializes the action. */
	virtual void DeinitializeAction();

	/** Used for Default actions. Primarily used in Gameplay Action Component. */
	bool RequestExecuteAction();

	/** Used for Triggered actions. Primarily used in Gameplay Action Component. */
	bool RequestTriggerAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel = -1, const FInstancedStruct& Payload = FInstancedStruct());

	void RequestReleaseInputID();
	
	/** Used to end the action. Primarily used in Gameplay Action Component. */
	void RequestEndAction();

	/** Used to cancel the action. Primarily used in Gameplay Action Component. */
	bool RequestCancelAction();
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	bool IsActive() const 
	{
		return bIsActive;
	}

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	bool CanBeCanceled() const 
	{
		return bCanBeCanceled;
	}
	
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
	
public:
	
	// Start of IGameplayTaskOwnerInterface
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;
	// End of IGameplayTaskOwnerInterface

	virtual UGameplayActionComponent* GetGameplayActionComponent_Implementation() const override
	{
		return GetActionComponent();
	}
	
	virtual UWorld* GetWorld() const override;
	
protected:
	
	/** Fires after action initialization, can be used for custom data initialization */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action", meta=(BlueprintInternalUseOnly, ForceAsFunction))
	void OnActionInitialize();

	/** Fires after action deinitialization, must be used if action have custom initialization data */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action", meta=(BlueprintInternalUseOnly, ForceAsFunction))
	void OnActionDeinitialize();
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	void EndAction();

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	void CancelAction();

	/** Applies the configured CostEffect and returns whether the cost was successfully paid. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Cost")
	bool CommitCost(int32 CostLevel = 1);

	/** Applies the configured CooldownEffect and returns whether the cooldown was successfully started. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Cooldown")
	bool CommitCooldown(int32 CooldownLevel = 1);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action")
	void SetCanBeCanceled(bool bNewCanBeCanceled) 
	{
		bCanBeCanceled = bNewCanBeCanceled;
	}
	
	/** Called when the action is executed (Standard actions). */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action")
	void OnExecuteAction();

	/** Event called when action is triggered with optional payload (Triggered actions). */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action")
	void OnActionTriggered(const FInstancedStruct& Payload);
	
	/** Called when Action Input ID is released */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action")
	void OnInputIDReleased();
	
	/** Called when the action ends. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action")
	void OnEndAction(bool bWasCanceled = false);

	/** Overridable function to check if the action can be executed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gameplay Action")
	bool CanExecuteAction() const;

protected:
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Action Info")
	int32 GetActionLevel() const 
	{
		return CachedActionLevel;
	}

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Action Info")
	AActor* GetOwnerActor() const 
	{
		return CachedActorInfo ? CachedActorInfo->OwnerActor.Get() : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Action Info")
	TArray<USkeletalMeshComponent*> GetSkeletalMeshComponents() const;

	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Action Info")
	UMovementComponent* GetMovementComponent() const 
	{ 
		return CachedActorInfo ? CachedActorInfo->MovementComponent.Get() : nullptr; 
	}
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay Action | Action Info")
	UGameplayActionComponent* GetActionComponent() const 
	{ 
		return CachedActorInfo ? CachedActorInfo->ActionComponent.Get() : nullptr; 
	}
	
protected:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Info", meta=(ShowOnlyInnerProperties))
	FGameplayActionInfoTags GameplayActionInfoTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay Action Info")
	EGameplayActionCancelPolicy CancelPolicy = EGameplayActionCancelPolicy::Block;

	/** Type of this action. Standard actions are added via AddGameplayAction. Triggered are executed on the fly. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Action Type")
	EGameplayActionType ActionType = EGameplayActionType::Default;

	/** Input ID associated with the action (Standard actions only). */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Input", meta=(EditCondition="ActionType == EGameplayActionType::Default"))
	int32 InputID;

	/** Cooldown effect to apply when the action is executed. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayActionEffect> CooldownEffect;

	/** Cost of the action. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Cost")
	TSubclassOf<UGameplayActionEffect> CostEffect;

private:

	/** Returns whether all currently active actions can be canceled. */
	bool CanCancelAllActions() const;

	/** Returns whether the owner has any of the BlockedByTags. */
	bool CheckBlockedTags() const;

	/** Returns whether the owner has all of the RequireTags. */
	bool CheckRequiredTags() const;

	/** Returns whether any other active action blocks this action via BlockOtherActions. */
	bool CheckBlockedActionTags() const;

	/** Returns whether the action's CostEffect can be applied successfully. */
	bool CanApplyCost() const;
	
	/** Ends all gameplay tasks owned by this action and clears the list. */
	void EndAllGameplayTasks();
	
private:

	int32 CachedActionLevel = -1;

	bool bIsActive = false;
	
	bool bCanBeCanceled = true;
	
	UPROPERTY()
	TArray<TObjectPtr<UGameplayTask>> GameplayTasks;

	TSharedPtr<FGameplayActionActorInfo> CachedActorInfo;
	
	friend class UGameplayActionComponent;
	friend class UGameplayActionFactory;
};
