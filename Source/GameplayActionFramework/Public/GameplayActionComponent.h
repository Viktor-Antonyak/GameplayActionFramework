// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionEffect.h"
#include "GameplayTagContainer.h"
#include "GameplayActionTypes.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayTasksComponent.h"
#include "GameplayTagStackableContainer.h"
#include "ActiveGameplayActionEffectsContainer.h"
#include "GameplayActionInterface.h"

// InstancedStruct.h path differs across engine versions (StructUtils plugin vs engine-native).
// __has_include avoids hardcoding a specific version boundary, which the maintainer cannot
// verify across all supported engine versions. If you hit a compile error here on your engine
// version, please open an issue with your exact UE version.
#if __has_include("StructUtils/InstancedStruct.h")
#include "StructUtils/InstancedStruct.h"
#else
#include "InstancedStruct.h"
#endif

#include "GameplayActionComponent.generated.h"

GAMEPLAYACTIONFRAMEWORK_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplayActionComponent, Log, All);

class UGameplayAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActionActivated, TSubclassOf<UGameplayAction>, Action);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActionEnded, TSubclassOf<UGameplayAction>, Action, bool, bWasCanceled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEffectAdded, TSubclassOf<UGameplayActionEffect>, Effect);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEffectRemoved, TSubclassOf<UGameplayActionEffect>, Effect);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttributeChanged, const FGameplayActionAttribute&, Attribute, float, NewValue, float, OldValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayTagsAdded, const FGameplayTagContainer&, GameplayTags);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayTagsRemoved, const FGameplayTagContainer&, GameplayTags);

/** Data for initializing specific attributes on the component */
USTRUCT(BlueprintType)
struct FAttributeInitializationData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Attribute")
	FGameplayActionAttribute Attribute;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Attribute")
	float InitialValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FActionInitializationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Action")
	TSubclassOf<UGameplayAction> Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Action")
	int32 Level = -1;
};

/** Component that manages Gameplay Actions and Attributes. */
UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionComponent : public UGameplayTasksComponent, public IGameplayActionInterface
{
	GENERATED_BODY()

public:
	
	explicit UGameplayActionComponent(const FObjectInitializer& ObjectInitializer);

	TSharedPtr<FGameplayActionActorInfo> GetGameplayActionActorInfo() const
	{
		return GameplayActionActorInfo;
	}

	// --- Debug Accessors ---

	const TArray<FGameplayActionSpec>& GetAddedActions() const
	{
		return AddedActions;
	}

	const TArray<UGameplayAction*>& GetActiveTriggeredActions() const
	{
		return ActiveTriggeredActions;
	}

	const FActiveGameplayActionEffectsContainer& GetActiveEffects() const
	{
		return ActiveEffects;
	}

	const TArray<UGameplayActionAttributeSet*>& GetAttributeSets() const
	{
		return AttributeSets;
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
	UFUNCTION(BlueprintPure, BlueprintCallable, Category="Gameplay Action")
	FGameplayTagContainer GetOwnedGameplayTags() const
	{
		return GameplayTagContainer.IsValid() ? GameplayTagContainer.GetAggregatedGameplayTags() : FGameplayTagContainer();
	}
	
	/** Add Gameplay Tags to component */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void AddOwnedGameplayTags(FGameplayTagContainer Tags, int32 Quantity = 1);
	
	/** Remove Gameplay Tags from component */
	UFUNCTION(BlueprintCallable, Category="Gameplay Action")
	void RemoveOwnedGameplayTags(FGameplayTagContainer Tags, int32 Quantity = 1);

	/** Returns how many sources are currently granting the tag (reference count). */
	UFUNCTION(BlueprintPure, Category="Gameplay Action")
	int32 GetOwnedGameplayTagCount(FGameplayTag Tag) const;

	// --- Attribute Management ---
	
	/** Returns specific owned Attribute Set from component */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Attribute")
	UGameplayActionAttributeSet* GetAttributeSetByClass(TSubclassOf<UGameplayActionAttributeSet> AttributeClass) const;
	
	/** Returns the float value of an attribute from the attribute sets. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	float GetAttributeValue(const FGameplayActionAttribute& Attribute) const;

	/** Returns the base value of an attribute. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	float GetAttributeBaseValue(const FGameplayActionAttribute& Attribute) const;

	/** Sets the float value of an attribute, triggering Pre/PostChange callbacks. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Attribute", meta = (AutoCreateRefTerm = "Attribute"))
	void SetAttributeValue(const FGameplayActionAttribute& Attribute, float NewValue);

	/** Adds an attribute set to this component. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Attribute")
	UGameplayActionAttributeSet* AddAttributeSet(TSubclassOf<UGameplayActionAttributeSet> AttributeSetClass);

	// --- Gameplay Effect Management ---

	/**
    * Applies a gameplay effect spec to the owner.
    * @param Spec The gameplay effect spec to apply.
    * @return Handle to the active effect, or invalid handle if application failed.
    */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	FActiveGameplayActionEffectHandle ApplyGameplayEffectSpecToSelf(const FGameplayActionEffectSpec& Spec);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	FActiveGameplayActionEffectHandle ApplyGameplayEffectToSelf(TSubclassOf<UGameplayActionEffect> Effect, int32 Level = 1, AActor* SourceActor = nullptr);

	/** Removes an active gameplay effect by its handle. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	bool RemoveActiveGameplayEffect(const FActiveGameplayActionEffectHandle& Handle);

	/** Returns whether the actor has an active gameplay effect spec. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	bool HasActiveEffectSpecHandle(const FActiveGameplayActionEffectHandle& Spec);

	/** Returns whether the actor has an active gameplay effect class. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	bool HasActiveEffect(TSubclassOf<UGameplayActionEffect> EffectClass);
	
	/** 
	 * Freezes/unfreezes an active effect's modifiers without removing it from the container.
	 * While inhibited, the effect's contribution to attributes is removed, but its handle, duration timer,
	 * and stack count remain intact.
	 *
	 * @note Inhibition applies to the ENTIRE active effect, including all of its stacks — there is no
	 * partial inhibition of individual stacks within a stacked effect. If an effect has StackCount = 3,
	 * inhibiting it removes the contribution of all 3 stacks at once; there's no way to inhibit e.g. only 1 of 3.
	 *
	 * @note Only affects effects that go through the attribute aggregator (Duration/Infinite policy).
	 * Instant effects have no ongoing modifiers to toggle and will be rejected — see the Warning log.
	 * Periodic effects (Period > 0) are separately checked in ExecutePeriodicEffect, which skips ticking
	 * while bIsInhibited is true.
	 *
	 * The caller is responsible for deciding when to inhibit/un-inhibit (e.g. based on gameplay tags,
	 * conditions, etc.) — the framework does not impose a trigger mechanism.
	 *
	 * @return false if the handle is invalid or already in the requested state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Effect")
	bool SetEffectInhibited(const FActiveGameplayActionEffectHandle& Handle, bool bInhibited);

	/** Returns whether the given active effect is currently inhibited. */
	UFUNCTION(BlueprintPure, Category = "Gameplay Effect")
	bool IsEffectInhibited(const FActiveGameplayActionEffectHandle& Handle) const;

	/** Draws debug info on canvas. */
	UE_DEPRECATED(1.6, "DisplayDebug is deprecated since GAF 1.6, use the Gameplay Debugger 'GAF' category instead.")
	void DisplayDebug(class UCanvas* Canvas, float& YL, float& YPos) const;
	
	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Gameplay Action")
	FActionActivated OnActionActivated;
	
	UPROPERTY(BlueprintAssignable, Category = "Gameplay Action")
	FActionEnded OnActionEnded;
	
	UPROPERTY(BlueprintAssignable, Category = "Gameplay Effect")
	FEffectAdded OnEffectAdded;
	
	UPROPERTY(BlueprintAssignable, Category = "Gameplay Effect")
	FEffectRemoved OnEffectRemoved;
	
	UPROPERTY(BlueprintAssignable, Category = "Gameplay Attribute")
	FAttributeChanged OnAttributeChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Gameplay Tag")
	FGameplayTagsAdded OnGameplayTagsAdded;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay Tag")
	FGameplayTagsRemoved OnGameplayTagsRemoved;
	
	// Interface
	virtual UGameplayActionComponent* GetGameplayActionComponent_Implementation() const override
	{
		return const_cast<UGameplayActionComponent*>(this);
	}
	
	/** Attribute sets to be created on BeginPlay */
	UPROPERTY(EditAnywhere, Category="Gameplay Attributes")
	TArray<TSubclassOf<UGameplayActionAttributeSet>> DefaultAttributes;
	
	/** Specific initial values for attributes (applied after set creation) */
	UPROPERTY(EditAnywhere, Category="Gameplay Attributes")
	TArray<FAttributeInitializationData> InitialAttributeValues;

	/** Actions to be initialized on BeginPlay */
	UPROPERTY(EditAnywhere, Category="Gameplay Actions")
	TArray<FActionInitializationData> InitialActions;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:

	/** Caches the owner actor's components into GameplayActionActorInfo. */
	void InitActorInfo();
	/** Creates and initializes the default attribute sets. */
	void InitAttributes();
	/** Adds and initializes the default actions. */
	void InitActions();

	FGameplayTagStackableContainer GameplayTagContainer;
	
	UPROPERTY()
	TArray<UGameplayAction*> ActiveActions;
	
	UPROPERTY()
	TArray<UGameplayAction*> ActiveTriggeredActions;
	
	UPROPERTY()
	TArray<FGameplayActionSpec> AddedActions;
	
	/** All active attribute sets managed by this component */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Attributes")
	TArray<UGameplayActionAttributeSet*> AttributeSets;
	
	/** Applies the active effect's modifiers to the attribute aggregators and starts its duration/period timers. */
	void ApplyEffectModifiers(const FActiveGameplayActionEffect& Effect);

	/** Removes the active effect's modifier contributions from the aggregators and cancels its timers. */
	void RevertEffectModifiers(const FActiveGameplayActionEffect& Effect);

	/** Called when a duration-based effect reaches its end; removes it from the container. */
	void OnEffectExpired(FActiveGameplayActionEffectHandle Handle);

	/** Called on each periodic tick of an effect; re-evaluates its execution. */
	void ExecutePeriodicEffect(FActiveGameplayActionEffectHandle Handle);

	UPROPERTY()
	FActiveGameplayActionEffectsContainer ActiveEffects;

	int32 NextEffectHandle = 1;

	TSharedPtr<FGameplayActionActorInfo> GameplayActionActorInfo;
};
