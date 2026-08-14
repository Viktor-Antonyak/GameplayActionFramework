// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "GameplayActionAttributeAggregator.h"
#include "GameplayActionAttributeSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayActionAttributeSet, Log, All);

class UGameplayActionAttributeSet;
class UGameplayActionComponent;

/**
 * Structure that holds the actual values of an attribute.
 * Contains both Base and Current values.
 */
USTRUCT(BlueprintType)
struct FGameplayActionAttributeData
{
	GENERATED_BODY()

public:
	
	FGameplayActionAttributeData()
		: BaseValue(0.f)
		, CurrentValue(0.f)
	{}

	FGameplayActionAttributeData(float DefaultValue)
		: BaseValue(DefaultValue)
		, CurrentValue(DefaultValue)
	{}

	float GetBaseValue() const { return BaseValue; }
	float GetCurrentValue() const { return CurrentValue; }

	void SetBaseValue(float NewValue) { BaseValue = NewValue; }
	void SetCurrentValue(float NewValue) { CurrentValue = NewValue; }

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	float BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	float CurrentValue;
};

/**
 * Describes a FGameplayAttributeData property inside an attribute set.
 * Used for identifying and accessing attributes in a generic way.
 */
USTRUCT(BlueprintType)
struct FGameplayActionAttribute
{
	GENERATED_BODY()

public:
	
	FGameplayActionAttribute() : Attribute(nullptr) {}
	GAMEPLAYACTIONFRAMEWORK_API FGameplayActionAttribute(FProperty* InProperty);

	bool IsValid() const { return Attribute.Get() != nullptr; }
	FProperty* GetUProperty() const { return Attribute.Get(); }

	GAMEPLAYACTIONFRAMEWORK_API FString GetAttributeName() const;

	/** Returns the class of the attribute set that owns this attribute */
	UClass* GetAttributeSetClass() const
	{
		return Attribute.Get() ? Cast<UClass>(Attribute.Get()->GetOwner<UObject>()) : nullptr;
	}

	bool operator==(const FGameplayActionAttribute& Other) const { return Attribute == Other.Attribute; }
	bool operator!=(const FGameplayActionAttribute& Other) const { return Attribute != Other.Attribute; }

	friend uint32 GetTypeHash(const FGameplayActionAttribute& InAttribute)
	{
		return PointerHash(InAttribute.Attribute.Get());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
	TFieldPath<FProperty> Attribute;
};

/**
 * Base class for all attribute sets in the Gameplay Action Framework.
 */
UCLASS(Blueprintable, DefaultToInstanced, ClassGroup=(Gameplay), Abstract)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionAttributeSet : public UObject
{
	GENERATED_BODY()

public:
	
	UGameplayActionAttributeSet();

	/** Called by the owning component right after creation. Not meant to be called manually. */
	void SetOwningActionComponent(UGameplayActionComponent* InComponent);

	/** Called just before an attribute's current value is changed. Return clamped value. */
	UFUNCTION(BlueprintNativeEvent, Category = "Attributes")
	float PreAttributeChange(const FGameplayActionAttribute& Attribute, float NewValue);
	virtual float PreAttributeChange_Implementation(const FGameplayActionAttribute& Attribute, float NewValue)
	{
		return NewValue;
	}
	
	/** Called just before an attribute's BASE value is changed (e.g. from Instant/Duration effects). Return clamped value. */
	UFUNCTION(BlueprintNativeEvent, Category = "Attributes")
	float PreAttributeBaseChange(const FGameplayActionAttribute& Attribute, float NewValue);
	virtual float PreAttributeBaseChange_Implementation(const FGameplayActionAttribute& Attribute, float NewValue)
	{
		return NewValue;
	}

	/** Called just after an attribute's current value is changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Attributes")
	void PostAttributeChange(const FGameplayActionAttribute& Attribute, float NewValue, float OldValue);
	virtual void PostAttributeChange_Implementation(const FGameplayActionAttribute& Attribute, float NewValue, float OldValue)
	{}

	/** Returns current numeric value of an attribute */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetNumericValue(const FGameplayActionAttribute& Attribute) const;

	/** Returns current base value of an attribute */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetBaseValue(const FGameplayActionAttribute& Attribute) const;

	/** Instant/periodic mutation - directly sets CurrentValue, bypassing the aggregator. */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetNumericValue(const FGameplayActionAttribute& Attribute, float NewValue);

	/** Instant/periodic mutation - directly sets BaseValue, bypassing the aggregator. */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetBaseValue(const FGameplayActionAttribute& Attribute, float NewValue);

	/** Initializes base and current value without callbacks (usually for setup) */
	void InitNumericValue(const FGameplayActionAttribute& Attribute, float NewValue);
	
	/** Registers a long-lived modifier contribution tied to a specific effect handle, and recalculates the attribute. */
	void ApplyModifier(const FGameplayActionAttribute& Attribute, const FActiveGameplayActionEffectHandle& SourceHandle, EGameplayActionEffectModOp Op, float Magnitude);
	
	/** Removes all modifier contributions from a specific effect handle, and recalculates the attribute. */
	void RemoveModifiersFromSource(const FGameplayActionAttribute& Attribute, const FActiveGameplayActionEffectHandle& SourceHandle);

protected:
	
	/** Recomputes CurrentValue for an attribute from BaseValue + all active aggregator modifiers, and broadcasts the change if it differs. */
	void RecalculateAttribute(const FGameplayActionAttribute& Attribute);
	
	/** Internal helper to find the data struct for an attribute property */
	FGameplayActionAttributeData* GetAttributeData(const FGameplayActionAttribute& Attribute);
	const FGameplayActionAttributeData* GetAttributeData(const FGameplayActionAttribute& Attribute) const;
	
	/** Broadcasts the attribute value change to the owning component and triggers PostAttributeChange. */
	void BroadcastAttributeChanges(const FGameplayActionAttribute& Attribute, float NewValue, float OldValue);

	UPROPERTY()
	TMap<FGameplayActionAttribute, FGameplayActionAttributeAggregator> Aggregators;
	
	UPROPERTY()
	TWeakObjectPtr<UGameplayActionComponent> OwningActionComponent;
};
