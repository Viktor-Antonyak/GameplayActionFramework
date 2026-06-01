// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAttributeSet.h"

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
	}
}

void UGameplayAttributeSet::SetBaseValue(const FGameplayAttribute& Attribute, float NewValue)
{
    FGameplayAttributeData* Data = GetAttributeData(Attribute);

    if (Data) 
    {
        float Delta = NewValue - Data->GetBaseValue();
        
        Data->SetBaseValue(NewValue);
        Data->SetCurrentValue(Data->GetCurrentValue() + Delta);
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
