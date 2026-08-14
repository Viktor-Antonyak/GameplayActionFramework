// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayActionFragment.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Blueprints/GameplayActionFragmentBlueprint.h"
#include "GameplayActionFrameworkFactories.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayActionFragment"

FText UAssetDefinition_GameplayActionFragment::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Action Fragment"));
}

FLinearColor UAssetDefinition_GameplayActionFragment::GetAssetColor() const
{
	return FLinearColor(0.4f, 1.0f, 1.0f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayActionFragment::GetAssetClass() const
{
	return UGameplayActionFragmentBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayActionFragment::GetAssetCategories() const
{
	static const auto Categories = {
		FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")),
		EAssetCategoryPaths::Blueprint
	};
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayActionFragment::GetClassTypeActions(const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayActionFragment::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameplayActionFragmentFactory* Factory = NewObject<UGameplayActionFragmentFactory>();

	if (InBlueprint)
	{
		Factory->ParentClass = InBlueprint->GeneratedClass;
	}

	return Factory;
}

namespace MenuExtension_GameplayActionFragment
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

			const UAssetDefinition_GameplayActionFragment* AssetDefinition = Cast<UAssetDefinition_GameplayActionFragment>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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

			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayActionFragmentBlueprint::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
				{
					if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayActionFragmentBlueprint::StaticClass()))
					{
						const TAttribute<FText> Label = LOCTEXT("GameplayActionFragment_NewDerivedBlueprint", "Create Child Fragment");
						const TAttribute<FText> ToolTip = LOCTEXT("GameplayActionFragment_NewDerivedBlueprintTooltip", "Creates a Child Gameplay Action Fragment Blueprint based on the current one.");
						const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint");

						FToolUIAction DeriveNewBlueprint;
						DeriveNewBlueprint.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewDerivedBlueprint, *SelectedAssetPtr);

						InSection.AddMenuEntry("CreateChildGameplayActionFragment", Label, ToolTip, Icon, DeriveNewBlueprint);
					}
				}
			}));
		}));
	});
}

#undef LOCTEXT_NAMESPACE