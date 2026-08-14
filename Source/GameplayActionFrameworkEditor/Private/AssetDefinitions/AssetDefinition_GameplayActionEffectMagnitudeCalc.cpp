// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayActionEffectMagnitudeCalc.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Blueprints/GameplayActionEffectMagnitudeCalcBlueprint.h"
#include "GameplayActionFrameworkFactories.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayEffectMagnitudeCalculation"

FText UAssetDefinition_GameplayActionEffectMagnitudeCalc::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Action Effect Magnitude Calculation"));
}

FLinearColor UAssetDefinition_GameplayActionEffectMagnitudeCalc::GetAssetColor() const
{
	return FLinearColor(0.8f, 0.4f, 1.0f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayActionEffectMagnitudeCalc::GetAssetClass() const
{
	return UGameplayActionEffectMagnitudeCalcBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayActionEffectMagnitudeCalc::GetAssetCategories() const
{
	static const auto Categories = {
		FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")),
		EAssetCategoryPaths::Blueprint
	};
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayActionEffectMagnitudeCalc::GetClassTypeActions(const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayActionEffectMagnitudeCalc::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameplayActionEffectMagnitudeCalculationFactory* Factory = NewObject<UGameplayActionEffectMagnitudeCalculationFactory>();

	if (InBlueprint)
	{
		Factory->ParentClass = InBlueprint->GeneratedClass;
	}

	return Factory;
}

//----------------------------------
// Menu Extension
//----------------------------------

namespace MenuExtension_GameplayEffectMagnitudeCalculation
{
	static void ExecuteNewDerivedBlueprint(const FToolMenuContext& MenuContext, const FAssetData SelectedBlueprintPtr)
	{
		if (UBlueprint* ParentBlueprint = Cast<UBlueprint>(SelectedBlueprintPtr.GetAsset()))
		{
			UClass* TargetParentClass = ParentBlueprint->GeneratedClass;

			if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(TargetParentClass))
			{
				return;
			}

			FString Name;
			FString PackageName;
			FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
			AssetToolsModule.Get().CreateUniqueAssetName(ParentBlueprint->GetOutermost()->GetName(), TEXT("_Child"), PackageName, Name);
			const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

			const UAssetDefinition_GameplayActionEffectMagnitudeCalc* AssetDefinition = Cast<UAssetDefinition_GameplayActionEffectMagnitudeCalc>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
			if (AssetDefinition)
			{
				UFactory* Factory = AssetDefinition->GetFactoryForBlueprintType(ParentBlueprint);

				FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				ContentBrowserModule.Get().CreateNewAsset(Name, PackagePath, ParentBlueprint->GetClass(), Factory);
			}
		}
	}

	static FDelayedAutoRegisterHelper DelayedAutoRegister(EDelayedRegisterRunPhase::EndOfEngineInit, []{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
		{
			FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);

			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayActionEffectMagnitudeCalcBlueprint::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
				{
					if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayActionEffectMagnitudeCalcBlueprint::StaticClass()))
					{
						const TAttribute<FText> Label = LOCTEXT("MagnitudeCalc_NewDerivedBlueprint", "Create Child Magnitude Calculation");
						const TAttribute<FText> ToolTip = LOCTEXT("MagnitudeCalc_NewDerivedBlueprintTooltip", "Creates a Child Gameplay Effect Magnitude Calculation Blueprint based on the current one.");
						const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint");

						FToolUIAction DeriveNewBlueprint;
						DeriveNewBlueprint.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewDerivedBlueprint, *SelectedAssetPtr);

						InSection.AddMenuEntry("CreateChildMagnitudeCalculation", Label, ToolTip, Icon, DeriveNewBlueprint);
					}
				}
			}));
		}));
	});
}

#undef LOCTEXT_NAMESPACE
