// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionAttributeSet.h"
#include "GameplayActionComponent.h"

DEFINE_LOG_CATEGORY(LogGameplayActionAttributeSet);

FGameplayActionAttribute::FGameplayActionAttribute(FProperty* InProperty)
{
	if (InProperty)
	{
		FStructProperty* StructProp = CastField<FStructProperty>(InProperty);
		if (StructProp && StructProp->Struct == FGameplayActionAttributeData::StaticStruct())
		{
			Attribute = InProperty;
		}
	}
}

FString FGameplayActionAttribute::GetAttributeName() const
{
	return Attribute.Get() != nullptr ? Attribute->GetName() : TEXT("None");
}

UGameplayActionAttributeSet::UGameplayActionAttributeSet()
{
}

void UGameplayActionAttributeSet::SetOwningActionComponent(UGameplayActionComponent* InComponent)
{
	OwningActionComponent = InComponent;
}

float UGameplayActionAttributeSet::GetNumericValue(const FGameplayActionAttribute& Attribute) const
{
	const FGameplayActionAttributeData* Data = GetAttributeData(Attribute);
	return Data ? Data->GetCurrentValue() : 0.0f;
}

float UGameplayActionAttributeSet::GetBaseValue(const FGameplayActionAttribute& Attribute) const
{
   	const FGameplayActionAttributeData* Data = GetAttributeData(Attribute);
	return Data ? Data->GetBaseValue() : 0.0f;
}

void UGameplayActionAttributeSet::SetNumericValue(const FGameplayActionAttribute& Attribute, float NewValue)
{
	FGameplayActionAttributeData* Data = GetAttributeData(Attribute);
	
	if (Data)
	{
		const float OldValue = Data->GetCurrentValue();
		
		NewValue = PreAttributeChange(Attribute, NewValue);
		
		Data->SetCurrentValue(NewValue);
		
		PostAttributeChange(Attribute, NewValue, OldValue);
		
		BroadcastAttributeChanges(Attribute, NewValue, OldValue);
	}
}

void UGameplayActionAttributeSet::SetBaseValue(const FGameplayActionAttribute& Attribute, float NewValue)
{
	FGameplayActionAttributeData* Data = GetAttributeData(Attribute);
	
	if (Data) 
	{
		NewValue = PreAttributeBaseChange(Attribute, NewValue);
		
		Data->SetBaseValue(NewValue);
		
		RecalculateAttribute(Attribute);
	}
}

void UGameplayActionAttributeSet::InitNumericValue(const FGameplayActionAttribute& Attribute, float NewValue)
{
	FGameplayActionAttributeData* Data = GetAttributeData(Attribute);

	if (Data)
	{
		Data->SetBaseValue(NewValue);
		Data->SetCurrentValue(NewValue);
	}
}

void UGameplayActionAttributeSet::ApplyModifier(const FGameplayActionAttribute& Attribute,
	const FActiveGameplayActionEffectHandle& SourceHandle, EGameplayActionEffectModOp Op, float Magnitude)
{
	if (!Attribute.IsValid() || !SourceHandle.IsValid())
	{
		return;
	}

	FGameplayActionAttributeAggregator& Aggregator = Aggregators.FindOrAdd(Attribute);
	Aggregator.AddModifier(SourceHandle, Op, Magnitude);

	RecalculateAttribute(Attribute);
}

void UGameplayActionAttributeSet::RemoveModifiersFromSource(const FGameplayActionAttribute& Attribute,
	const FActiveGameplayActionEffectHandle& SourceHandle)
{
	FGameplayActionAttributeAggregator* Aggregator = Aggregators.Find(Attribute);
	
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

void UGameplayActionAttributeSet::RecalculateAttribute(const FGameplayActionAttribute& Attribute)
{
	FGameplayActionAttributeData* Data = GetAttributeData(Attribute);
	
	if (!Data)
	{
		return;
	}

	const float OldValue = Data->GetCurrentValue();

	const FGameplayActionAttributeAggregator* Aggregator = Aggregators.Find(Attribute);
	
	const float RawNewValue = Aggregator ? Aggregator->CalculateCurrentValue(Data->GetBaseValue()) : Data->GetBaseValue();

	const float ClampedValue = PreAttributeChange(Attribute, RawNewValue);
	
	Data->SetCurrentValue(ClampedValue);

	PostAttributeChange(Attribute, ClampedValue, OldValue);
	
	BroadcastAttributeChanges(Attribute, ClampedValue, OldValue);
}

FGameplayActionAttributeData* UGameplayActionAttributeSet::GetAttributeData(const FGameplayActionAttribute& Attribute)
{
	FProperty* Prop = Attribute.GetUProperty();
	if (Prop)
	{
		FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp)
		{
			return StructProp->ContainerPtrToValuePtr<FGameplayActionAttributeData>(this);
		}
	}
	return nullptr;
}

const FGameplayActionAttributeData* UGameplayActionAttributeSet::GetAttributeData(const FGameplayActionAttribute& Attribute) const
{
	FProperty* Prop = Attribute.GetUProperty();
	if (Prop)
	{
		FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp)
		{
			return StructProp->ContainerPtrToValuePtr<FGameplayActionAttributeData>(this);
		}
	}
	return nullptr;
}

void UGameplayActionAttributeSet::BroadcastAttributeChanges(const FGameplayActionAttribute& Attribute, float NewValue, float OldValue)
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
