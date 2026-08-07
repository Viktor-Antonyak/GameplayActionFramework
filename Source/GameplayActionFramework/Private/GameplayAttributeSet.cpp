// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAttributeSet.h"
#include "GameplayActionComponent.h"

DEFINE_LOG_CATEGORY(LogGameplayAttributeSet);

FGameplayAttribute::FGameplayAttribute(FProperty* InProperty)
{
	if (InProperty)
	{
		FStructProperty* StructProp = CastField<FStructProperty>(InProperty);
		if (StructProp && StructProp->Struct == FGameplayAttributeData::StaticStruct())
		{
			Attribute = InProperty;
		}
	}
}

FString FGameplayAttribute::GetAttributeName() const
{
	return Attribute.Get() != nullptr ? Attribute->GetName() : TEXT("None");
}

UGameplayAttributeSet::UGameplayAttributeSet()
{
}

void UGameplayAttributeSet::SetOwningActionComponent(UGameplayActionComponent* InComponent)
{
	OwningActionComponent = InComponent;
}

float UGameplayAttributeSet::GetNumericValue(const FGameplayAttribute& Attribute) const
{
	const FGameplayAttributeData* Data = GetAttributeData(Attribute);
	return Data ? Data->GetCurrentValue() : 0.0f;
}

float UGameplayAttributeSet::GetBaseValue(const FGameplayAttribute& Attribute) const
{
   	const FGameplayAttributeData* Data = GetAttributeData(Attribute);
	return Data ? Data->GetBaseValue() : 0.0f;
}

void UGameplayAttributeSet::SetNumericValue(const FGameplayAttribute& Attribute, float NewValue)
{
	FGameplayAttributeData* Data = GetAttributeData(Attribute);
	
	if (Data)
	{
		const float OldValue = Data->GetCurrentValue();
		
		NewValue = PreAttributeChange(Attribute, NewValue);
		
		Data->SetCurrentValue(NewValue);
		
		PostAttributeChange(Attribute, NewValue, OldValue);
		
		BroadcastAttributeChanges(Attribute, NewValue, OldValue);
	}
}

void UGameplayAttributeSet::SetBaseValue(const FGameplayAttribute& Attribute, float NewValue)
{
	FGameplayAttributeData* Data = GetAttributeData(Attribute);
	
	if (Data) 
	{
		NewValue = PreAttributeBaseChange(Attribute, NewValue);
		
		Data->SetBaseValue(NewValue);
		
		RecalculateAttribute(Attribute);
	}
}

void UGameplayAttributeSet::InitNumericValue(const FGameplayAttribute& Attribute, float NewValue)
{
	FGameplayAttributeData* Data = GetAttributeData(Attribute);

	if (Data)
	{
		Data->SetBaseValue(NewValue);
		Data->SetCurrentValue(NewValue);
	}
}

void UGameplayAttributeSet::ApplyModifier(const FGameplayAttribute& Attribute,
	const FActiveGameplayEffectHandle& SourceHandle, EGameplayModOp Op, float Magnitude)
{
	if (!Attribute.IsValid() || !SourceHandle.IsValid())
	{
		return;
	}

	FGameplayAttributeAggregator& Aggregator = Aggregators.FindOrAdd(Attribute);
	Aggregator.AddModifier(SourceHandle, Op, Magnitude);

	RecalculateAttribute(Attribute);
}

void UGameplayAttributeSet::RemoveModifiersFromSource(const FGameplayAttribute& Attribute,
	const FActiveGameplayEffectHandle& SourceHandle)
{
	FGameplayAttributeAggregator* Aggregator = Aggregators.Find(Attribute);
	
	if (!Aggregator)
	{
		return;
	}

	Aggregator->RemoveModifiersFromSource(SourceHandle);

	if (Aggregator->IsEmpty())
	{
		Aggregators.Remove(Attribute);
	}

	RecalculateAttribute(Attribute);
}

void UGameplayAttributeSet::RecalculateAttribute(const FGameplayAttribute& Attribute)
{
	FGameplayAttributeData* Data = GetAttributeData(Attribute);
	
	if (!Data)
	{
		return;
	}

	const float OldValue = Data->GetCurrentValue();

	const FGameplayAttributeAggregator* Aggregator = Aggregators.Find(Attribute);
	
	const float RawNewValue = Aggregator ? Aggregator->CalculateCurrentValue(Data->GetBaseValue()) : Data->GetBaseValue();

	const float ClampedValue = PreAttributeChange(Attribute, RawNewValue);
	
	Data->SetCurrentValue(ClampedValue);

	PostAttributeChange(Attribute, ClampedValue, OldValue);
	
	BroadcastAttributeChanges(Attribute, ClampedValue, OldValue);
}

FGameplayAttributeData* UGameplayAttributeSet::GetAttributeData(const FGameplayAttribute& Attribute)
{
	FProperty* Prop = Attribute.GetUProperty();
	if (Prop)
	{
		FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp)
		{
			return StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
		}
	}
	return nullptr;
}

const FGameplayAttributeData* UGameplayAttributeSet::GetAttributeData(const FGameplayAttribute& Attribute) const
{
	FProperty* Prop = Attribute.GetUProperty();
	if (Prop)
	{
		FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp)
		{
			return StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
		}
	}
	return nullptr;
}

void UGameplayAttributeSet::BroadcastAttributeChanges(const FGameplayAttribute& Attribute, float NewValue, float OldValue)
{
	if (FMath::IsNearlyEqual(NewValue, OldValue))
	{
		return;
	}

	if (OwningActionComponent.Get())
	{
		OwningActionComponent->OnAttributeChanged.Broadcast(Attribute, NewValue, OldValue);
	}
}
