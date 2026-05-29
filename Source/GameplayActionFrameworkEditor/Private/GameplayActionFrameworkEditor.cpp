// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionFrameworkEditor.h"
#include "GameplayAttributeCustomization.h"
#include "GameplayAttributePinCustomization.h"
#include "EdGraphUtilities.h"
#include "GameplayAction.h"
#include "GameplayAttributeSet.h"
#include "PropertyEditorModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"

#define LOCTEXT_NAMESPACE "FGameplayActionFrameworkEditorModule"

class FGameplayAttributePinFactory : public FGraphPanelPinFactory
{
public:
	virtual TSharedPtr<SGraphPin> CreatePin(class UEdGraphPin* Pin) const override
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
		{
			const UScriptStruct* PinStruct = Cast<UScriptStruct>(Pin->PinType.PinSubCategoryObject.Get());
			if (PinStruct && PinStruct->GetName() == TEXT("GameplayAttribute"))
			{
				return SNew(SGraphPinGameplayAttribute, Pin);
			}
		}
		return nullptr;
	}
};

TSharedPtr<FGameplayAttributePinFactory> AttributePinFactory;
EAssetTypeCategories::Type GAFAssetCategory;

void FGameplayActionFrameworkEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	AttributePinFactory = MakeShareable(new FGameplayAttributePinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(AttributePinFactory);
	
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"GameplayAttribute",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayAttributeCustomization::MakeInstance)
	);
}

void FGameplayActionFrameworkEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("GameplayAttribute");
	}

	if (AttributePinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(AttributePinFactory);
		AttributePinFactory.Reset();
	}

	// Unregister asset actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	RegisteredAssetTypeActions.Empty();
}

void FGameplayActionFrameworkEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGameplayActionFrameworkEditorModule, GameplayActionFrameworkEditor)
