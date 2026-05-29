// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayAction.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Blueprints/GameplayActionBlueprint.h"
#include "GameplayActionFrameworkFactories.h"
#include "IClassTypeActions.h"
#include "IContentBrowserSingleton.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayAction"

FText UAssetDefinition_GameplayAction::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Action"));
}

FLinearColor UAssetDefinition_GameplayAction::GetAssetColor() const
{
	return FLinearColor(0.0f, 1.0f, 1.0f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayAction::GetAssetClass() const
{
	return UGameplayActionBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayAction::GetAssetCategories() const
{
	static const auto Categories = { FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")), EAssetCategoryPaths::Blueprint };
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayAction::GetClassTypeActions(const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayAction::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
    UGameplayActionFactory* Factory = NewObject<UGameplayActionFactory>();

    if (InBlueprint)
    {
        Factory->ParentClass = InBlueprint->GeneratedClass;
    }

	return Factory;
}

//----------------------------------
// Menu Extension
//----------------------------------

namespace MenuExtension_GameplayAction
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

            const UAssetDefinition_GameplayAction* AssetDefinition = Cast<UAssetDefinition_GameplayAction>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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
            UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayActionBlueprint::StaticClass());

            FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
            Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
            {
                if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
                {
                    if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayActionBlueprint::StaticClass()))
                    {
                        const TAttribute<FText> Label = LOCTEXT("GameplayAction_NewDerivedBlueprint", "Create Child Blueprint Class");
                        const TAttribute<FText> ToolTip = LOCTEXT("GameplayAction_NewDerivedBlueprintTooltip", "Creates a Child Blueprint Class based on the current Gameplay Action.");
                        const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint");

                        FToolUIAction DeriveNewBlueprint;
                        DeriveNewBlueprint.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewDerivedBlueprint, *SelectedAssetPtr);

                        InSection.AddMenuEntry("CreateChildGameplayAction", Label, ToolTip, Icon, DeriveNewBlueprint);
                    }
                }
            }));
        }));
    });
}

#undef LOCTEXT_NAMESPACE
