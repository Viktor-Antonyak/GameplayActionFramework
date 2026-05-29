// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

/** Internal structure for combo box items - moved outside for reuse in pins */
struct FAttributeItem
{
	FProperty* Property;
	FString DisplayString;

	FAttributeItem(FProperty* InProp, const FString& InDisplay)
		: Property(InProp), DisplayString(InDisplay) {}
};

/**
 * Customization for FGameplayAttribute struct to allow picking attributes from a dropdown.
 */
class FGameplayAttributeCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	typedef FAttributeItem FAttributeItemType;

private:
	TSharedPtr<IPropertyHandle> AttributePropertyHandle;
	TArray<TSharedPtr<FAttributeItem>> AttributeOptions;

	void OnAttributeSelected(TSharedPtr<FAttributeItem> SelectedItem, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> OnGenerateWidget(TSharedPtr<FAttributeItem> Item);
	FText GetSelectedText() const;
};
