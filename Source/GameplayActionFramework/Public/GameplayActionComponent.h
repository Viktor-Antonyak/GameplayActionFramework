// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GameplayActionActorInfo.h"
#include "GameplayActionSpec.h"
#include "GameplayAttributeSet.h"
#include "Templates/SharedPointer.h"
#include "Templates/SubclassOf.h"
#include "GameplayTasksComponent.h"
#include "InstancedStruct.h"

#include "GameplayActionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayActionComponent, Log, All);

class UGameplayAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActionActivated, TSubclassOf<UGameplayAction>, Action);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActionEnded, TSubclassOf<UGameplayAction>, Action, bool, bWasCanceled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayTagsAdded, const FGameplayTagContainer&, GameplayTags);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayTagsRemoved, const FGameplayTagContainer&, GameplayTags);

/** Data for initializing specific attributes on the component */
USTRUCT(BlueprintType)
struct FAttributeInitializationData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Attribute")
	FGameplayAttribute Attribute;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Attribute")
	float InitialValue = 0.0f;
};

/** Component that manages Gameplay Actions and Attributes. */
UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionComponent : public UGameplayTasksComponent
{
	GENERATED_BODY()

public:
	explicit UGameplayActionComponent(const FObjectInitializer& ObjectInitializer);

	TSharedPtr<FGameplayActionActorInfo> GetGameplayActionActorInfo() const
	{
		return GameplayActionActorInfo;
	}

	// --- Action Management ---
	
	/** Adds Gameplay Action to available actions */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	FGameplayActionSpec AddGameplayAction(const TSubclassOf<UGameplayAction> Action, const int32 Level = -1);
	
	/** Removes Gameplay Action from available actions */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void RemoveGameplayAction(const FGameplayActionSpec& ActionSpec);
	
	/** Activate Action by Action Spec */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	bool TryActivateActionBySpec(const FGameplayActionSpec& ActionSpec);
	
	/** Activate Action by Gameplay Tag */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	bool TryActivateActionsByTag(UPARAM(meta=(GameplayTagFilter="GameplayEventTagsCategory")) FGameplayTagContainer ActionTags);
	
	/** Activate Action by pressing Input ID defined in the action. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void PressInputID(int32 InputID);
	
	/** End Action by releasing Input ID defined in the action. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void ReleaseInputID(int32 InputID);
	
	/** Spawns and triggers a new action of the specified class with payload. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void TriggerActionByClass(TSubclassOf<UGameplayAction> ActionClass, const FInstancedStruct& Payload, int32 ActionLevel = -1);
	
	/** Returns array of active actions. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	TArray<UGameplayAction*> GetActiveActions() const { return ActiveActions; }
	
	// Default Actions
	void AddActiveAction(UGameplayAction* Action);
	void RemoveActiveAction(UGameplayAction* Action, bool bWasCanceled = false);

	// Triggered Actions
	void AddTriggeredAction(UGameplayAction* Action);
	void RemoveTriggeredAction(UGameplayAction* Action, bool bWasCanceled = false);
	
	// --- Tag Management ---
	
	/** Returns owned Gameplay Tags from component */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gameplay Action")
	FGameplayTagContainer GetOwnedGameplayTags() const { return GameplayTagContainer.IsValid() ? *GameplayTagContainer.Get() : FGameplayTagContainer(); }
	
	/** Add Gameplay Tags to component */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void AddOwnedGameplayTags(UPARAM(meta=(GameplayTagFilter="GameplayEventTagsCategory")) FGameplayTagContainer Tags);
	
	/** Remove Gameplay Tags from component */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void RemoveOwnedGameplayTags(UPARAM(meta=(GameplayTagFilter="GameplayEventTagsCategory")) FGameplayTagContainer Tags);

	// --- Attribute Management ---
	
	/** Returns specific owned Attribute Set from component */
	UFUNCTION(BlueprintCallable, Category="Gameplay Attribute")
	UGameplayAttributeSet* GetAttributeSetByClass(TSubclassOf<UGameplayAttributeSet> AttributeClass) const;
	
	/** Returns the float value of an attribute from the attribute sets. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	float GetAttributeValue(const FGameplayAttribute& Attribute) const;

	/** Returns the base value of an attribute. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	float GetAttributeBaseValue(const FGameplayAttribute& Attribute) const;

	/** Sets the float value of an attribute, triggering Pre/PostChange callbacks. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	void SetAttributeValue(const FGameplayAttribute& Attribute, float NewValue);

	/** Adds an attribute set to this component. */
	UFUNCTION(BlueprintCallable, Category="Gameplay Attribute")
	UGameplayAttributeSet* AddAttributeSet(TSubclassOf<UGameplayAttributeSet> AttributeSetClass);

	// --- Gameplay Effect Management ---

	/**
    * Applies a gameplay effect spec to the owner.
    * @param Spec The gameplay effect spec to apply.
    * @return Handle to the active effect, or invalid handle if application failed.
    */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	FActiveGameplayEffectHandle ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& Spec);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	FActiveGameplayEffectHandle ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect);

	/** Removes an active gameplay effect by its handle. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	bool RemoveActiveGameplayEffect(const FActiveGameplayEffectHandle& Handle);

	/** Returns whether the actor has an active gameplay effect spec. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	bool HasActiveEffectSpecHandle(const FActiveGameplayEffectHandle& Spec);

	/** Returns whether the actor has an active gameplay effect class. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	bool HasActiveEffect(TSubclassOf<UGameplayEffect> EffectClass);
	
	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category="Gameplay Action")
	FActionActivated OnActionActivated;
	
	UPROPERTY(BlueprintAssignable, Category="Gameplay Action")
	FActionEnded OnActionEnded;

	UPROPERTY(BlueprintAssignable, Category="Gameplay Action")
	FGameplayTagsAdded OnGameplayTagsAdded;

	UPROPERTY(BlueprintAssignable, Category="Gameplay Action")
	FGameplayTagsRemoved OnGameplayTagsRemoved;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitActorInfo();
	void InitAttributes();

	TSharedPtr<FGameplayTagContainer> GameplayTagContainer;
	
	UPROPERTY()
	TArray<UGameplayAction*> ActiveActions;
	
	UPROPERTY()
	TArray<UGameplayAction*> ActiveTriggeredActions;
	
	UPROPERTY()
	TArray<FGameplayActionSpec> AddedActions;
	
	/** Attribute sets to be created on BeginPlay */
	UPROPERTY(EditAnywhere, Category="Gameplay Attributes")
	TArray<TSubclassOf<UGameplayAttributeSet>> DefaultAttributes;
	
	/** Specific initial values for attributes (applied after set creation) */
	UPROPERTY(EditAnywhere, Category="Gameplay Attributes")
	TArray<FAttributeInitializationData> InitialAttributeValues;
	
	/** All active attribute sets managed by this component */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Attributes")
	TArray<UGameplayAttributeSet*> AttributeSets;

	void RevertEffectModifiers(const FActiveGameplayEffect& Effect);

	void OnEffectExpired(FActiveGameplayEffectHandle Handle);

	void ExecutePeriodicEffect(FActiveGameplayEffectHandle Handle);

	FActiveGameplayEffectHandle FindStackableEffect(const FGameplayEffectSpec& Spec) const;

	UPROPERTY()
	TArray<FActiveGameplayEffect> ActiveEffects;

	int32 NextEffectHandle = 1;

	TSharedPtr<FGameplayActionActorInfo> GameplayActionActorInfo;
};
