// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "GameplayAttributeCustomization.h" 

class SGraphPinGameplayAttribute : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SGraphPinGameplayAttribute) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

protected:
	// This replaces the default value widget (the text box) with our custom combo box
	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;

	TArray<TSharedPtr<FAttributeItem>> AttributeOptions;
};
