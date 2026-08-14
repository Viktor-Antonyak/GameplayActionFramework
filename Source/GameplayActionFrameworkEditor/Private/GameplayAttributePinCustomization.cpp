// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAttributePinCustomization.h"
#include "GameplayActionAttributeSet.h"
#include "HAL/Platform.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "UObject/UObjectIterator.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "ScopedTransaction.h"

void SGraphPinGameplayAttribute::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget> SGraphPinGameplayAttribute::GetDefaultValueWidget()
{
	// Populate options
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
		if (Class->IsChildOf(UGameplayActionAttributeSet::StaticClass()) && 
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
		{
			for (TFieldIterator<FProperty> PropIt(Class, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
			{
				FProperty* Property = *PropIt;
				
				// Check if the property is FGameplayActionAttributeData
				FStructProperty* StructProp = CastField<FStructProperty>(Property);
				if (StructProp && StructProp->Struct == FGameplayActionAttributeData::StaticStruct())
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

	return SNew(SComboBox<TSharedPtr<FAttributeItem>>)
		.OptionsSource(&AttributeOptions)
		.Visibility_Lambda([this]() -> EVisibility
        {
            if (GraphPinObj && GraphPinObj->LinkedTo.Num() > 0)
            {
                return EVisibility::Collapsed;
            }
            return EVisibility::Visible;
        })
		.OnGenerateWidget_Lambda([](TSharedPtr<FAttributeItem> Item)
		{
			return SNew(STextBlock).Text(FText::FromString(Item->DisplayString));
		})
		.OnSelectionChanged_Lambda([this](TSharedPtr<FAttributeItem> SelectedItem, ESelectInfo::Type SelectInfo)
        {
            if (SelectedItem.IsValid() && GraphPinObj)
            {
                FGameplayActionAttribute NewAttributeValue;
                if (SelectedItem->Property)
                {
                    NewAttributeValue = FGameplayActionAttribute(SelectedItem->Property);
                }
        
                FString ValueString;
                UScriptStruct* StructType = FGameplayActionAttribute::StaticStruct();
                StructType->ExportText(ValueString, &NewAttributeValue, &NewAttributeValue, nullptr, PPF_None, nullptr);
        
                if (GraphPinObj->GetDefaultAsString() != ValueString)
                {
                    const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeAttributePinValue", "Change Attribute Pin Value"));
        
                    GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, ValueString);
        
                    if (UEdGraphNode* OwningNode = GraphPinObj->GetOwningNode())
                    {
                        OwningNode->PinDefaultValueChanged(GraphPinObj);
                    }
        
                    Invalidate(EInvalidateWidgetReason::Layout);
                }
            }
        })
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
            {
            	FString DefaultString = GraphPinObj->GetDefaultAsString();
            	if (DefaultString.IsEmpty())
            	{
            		return FText::FromString(TEXT("None"));
            	}
            
            	FGameplayActionAttribute ParsedAttribute;
            	UScriptStruct* StructType = FGameplayActionAttribute::StaticStruct();
            	StructType->ImportText(*DefaultString, &ParsedAttribute, nullptr, PPF_None, nullptr, StructType->GetName());
            
            	if (!ParsedAttribute.IsValid())
            	{
            		return FText::FromString(TEXT("None"));
            	}
            
            	FString DisplayClassName = ParsedAttribute.GetAttributeSetClass() ? ParsedAttribute.GetAttributeSetClass()->GetName() : TEXT("");
            	DisplayClassName.RemoveFromStart(TEXT("U"));
            	DisplayClassName.RemoveFromEnd(TEXT("_C"));
            
            	return FText::FromString(FString::Printf(TEXT("%s.%s"), *DisplayClassName, *ParsedAttribute.GetAttributeName()));
            })
		];
}