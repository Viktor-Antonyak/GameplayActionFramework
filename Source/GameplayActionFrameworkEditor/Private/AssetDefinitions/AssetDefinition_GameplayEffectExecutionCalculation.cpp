// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayEffectExecutionCalculation.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Blueprints/GameplayEffectExecutionCalculationBlueprint.h"
#include "GameplayActionFrameworkFactories.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayEffectExecutionCalculation"

FText UAssetDefinition_GameplayEffectExecutionCalculation::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Effect Execution Calculation"));
}

FLinearColor UAssetDefinition_GameplayEffectExecutionCalculation::GetAssetColor() const
{
	return FLinearColor(1.0f, 0.6f, 0.2f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayEffectExecutionCalculation::GetAssetClass() const
{
	return UGameplayEffectExecutionCalculationBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayEffectExecutionCalculation::GetAssetCategories() const
{
	static const auto Categories = {
		FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")),
		EAssetCategoryPaths::Blueprint
	};
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayEffectExecutionCalculation::GetClassTypeActions(const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayEffectExecutionCalculation::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameplayEffectExecutionCalculationFactory* Factory = NewObject<UGameplayEffectExecutionCalculationFactory>();

	if (InBlueprint)
	{
		Factory->ParentClass = InBlueprint->GeneratedClass;
	}

	return Factory;
}

//----------------------------------
// Menu Extension
//----------------------------------

namespace MenuExtension_GameplayEffectExecutionCalculation
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

			const UAssetDefinition_GameplayEffectExecutionCalculation* AssetDefinition = Cast<UAssetDefinition_GameplayEffectExecutionCalculation>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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

			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayEffectExecutionCalculationBlueprint::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
				{
					if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayEffectExecutionCalculationBlueprint::StaticClass()))
					{
						const TAttribute<FText> Label = LOCTEXT("ExecCalc_NewDerivedBlueprint", "Create Child Execution Calculation");
						const TAttribute<FText> ToolTip = LOCTEXT("ExecCalc_NewDerivedBlueprintTooltip", "Creates a Child Gameplay Effect Execution Calculation Blueprint based on the current one.");
						const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint");

						FToolUIAction DeriveNewBlueprint;
						DeriveNewBlueprint.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewDerivedBlueprint, *SelectedAssetPtr);

						InSection.AddMenuEntry("CreateChildExecutionCalculation", Label, ToolTip, Icon, DeriveNewBlueprint);
					}
				}
			}));
		}));
	});
}

#undef LOCTEXT_NAMESPACE
