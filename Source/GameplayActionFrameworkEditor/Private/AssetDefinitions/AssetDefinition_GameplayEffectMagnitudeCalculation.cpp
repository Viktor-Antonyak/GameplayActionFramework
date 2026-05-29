// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayEffectMagnitudeCalculation.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Blueprints/GameplayEffectMagnitudeCalculationBlueprint.h"
#include "GameplayActionFrameworkFactories.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayEffectMagnitudeCalculation"

FText UAssetDefinition_GameplayEffectMagnitudeCalculation::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Effect Magnitude Calculation"));
}

FLinearColor UAssetDefinition_GameplayEffectMagnitudeCalculation::GetAssetColor() const
{
	return FLinearColor(0.8f, 0.4f, 1.0f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayEffectMagnitudeCalculation::GetAssetClass() const
{
	return UGameplayEffectMagnitudeCalculationBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayEffectMagnitudeCalculation::GetAssetCategories() const
{
	static const auto Categories = {
		FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")),
		EAssetCategoryPaths::Blueprint
	};
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayEffectMagnitudeCalculation::GetClassTypeActions(const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayEffectMagnitudeCalculation::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameplayEffectMagnitudeCalculationFactory* Factory = NewObject<UGameplayEffectMagnitudeCalculationFactory>();

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

			const UAssetDefinition_GameplayEffectMagnitudeCalculation* AssetDefinition = Cast<UAssetDefinition_GameplayEffectMagnitudeCalculation>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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

			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayEffectMagnitudeCalculationBlueprint::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
				{
					if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayEffectMagnitudeCalculationBlueprint::StaticClass()))
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
