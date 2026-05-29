// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAttributeCustomization.h"
#include "GameplayAttributeSet.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "HAL/Platform.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "GameplayAttributeCustomization"

TSharedRef<IPropertyTypeCustomization> FGameplayAttributeCustomization::MakeInstance()
{
	return MakeShareable(new FGameplayAttributeCustomization());
}

void FGameplayAttributeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Get the handle to the "Attribute" property inside FGameplayAttribute
	AttributePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, Attribute));

	if (!AttributePropertyHandle.IsValid())
	{
		return;
	}

	// Populate AttributeOptions
	AttributeOptions.Empty();
	AttributeOptions.Add(MakeShareable(new FAttributeItem(nullptr, TEXT("None"))));

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		
		// Filter out garbage classes created by Live Coding / Hot Reload (SKEL_, REINST_)
		FString ClassName = Class->GetName();
		if (ClassName.StartsWith(TEXT("SKEL_")) || ClassName.StartsWith(TEXT("REINST_")) || ClassName.StartsWith(TEXT("TRASH_")))
		{
			continue;
		}

		// Only consider valid GameplayAttributeSet classes
		if (Class->IsChildOf(UGameplayAttributeSet::StaticClass()) && 
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
		{
			for (TFieldIterator<FProperty> PropIt(Class, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
			{
				FProperty* Property = *PropIt;
				
				// Check if the property is FGameplayAttributeData
				FStructProperty* StructProp = CastField<FStructProperty>(Property);
				if (StructProp && StructProp->Struct == FGameplayAttributeData::StaticStruct())
				{
					FString DisplayClassName = Class->GetName();
					DisplayClassName.RemoveFromStart(TEXT("U"));
					DisplayClassName.RemoveFromEnd(TEXT("_C"));
					FString DisplayName = FString::Printf(TEXT("%s.%s"), *DisplayClassName, *Property->GetName());
					AttributeOptions.Add(MakeShareable(new FAttributeItem(Property, DisplayName)));
				}
			}
		}
	}

	// Create the widget
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(300.f)
	[
		SNew(SComboBox<TSharedPtr<FAttributeItem>>)
		.OptionsSource(&AttributeOptions)
		.OnGenerateWidget(this, &FGameplayAttributeCustomization::OnGenerateWidget)
		.OnSelectionChanged(this, &FGameplayAttributeCustomization::OnAttributeSelected)
		[
			SNew(STextBlock)
			.Text(this, &FGameplayAttributeCustomization::GetSelectedText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
}

void FGameplayAttributeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

void FGameplayAttributeCustomization::OnAttributeSelected(TSharedPtr<FAttributeItem> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid() && AttributePropertyHandle.IsValid())
	{
		if (SelectedItem->Property)
		{
			// Setting TFieldPath via string "ClassPath:PropertyName"
			FString ValueString = FString::Printf(TEXT("%s:%s"), 
				*SelectedItem->Property->GetOwner<UObject>()->GetPathName(), 
				*SelectedItem->Property->GetName());
			
			AttributePropertyHandle->SetValueFromFormattedString(ValueString);
		}
		else
		{
			AttributePropertyHandle->SetValueFromFormattedString(TEXT("None"));
		}
	}
}

TSharedRef<SWidget> FGameplayAttributeCustomization::OnGenerateWidget(TSharedPtr<FAttributeItem> Item)
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item->DisplayString))
		.Font(IDetailLayoutBuilder::GetDetailFont());
}

FText FGameplayAttributeCustomization::GetSelectedText() const
{
	if (AttributePropertyHandle.IsValid())
	{
		void* Data = nullptr;
		if (AttributePropertyHandle->GetValueData(Data) == FPropertyAccess::Success && Data)
		{
			TFieldPath<FProperty>* FieldPath = (TFieldPath<FProperty>*)Data;
			if (FieldPath && FieldPath->Get() != nullptr)
			{
				FProperty* Prop = FieldPath->Get();
				UClass* OwnerClass = Cast<UClass>(Prop->GetOwner<UObject>());
				FString ClassName = OwnerClass ? OwnerClass->GetName() : TEXT("Unknown");
				ClassName.RemoveFromStart(TEXT("U"));
				ClassName.RemoveFromEnd(TEXT("_C"));
				return FText::FromString(FString::Printf(TEXT("%s.%s"), *ClassName, *Prop->GetName()));
			}
		}
	}
	return LOCTEXT("None", "None");
}

#undef LOCTEXT_NAMESPACE
