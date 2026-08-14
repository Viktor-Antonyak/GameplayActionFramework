// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayActionAttributeSet.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "GameplayActionFrameworkFactories.h"
#include "Blueprints/GameplayActionAttributeSetBlueprint.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayAttributeSet"

FText UAssetDefinition_GameplayActionAttributeSet::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Attribute Set"));
}

FLinearColor UAssetDefinition_GameplayActionAttributeSet::GetAssetColor() const
{
	return FLinearColor(1.0f, 0.0f, 0.4f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayActionAttributeSet::GetAssetClass() const
{
	return UGameplayActionAttributeSetBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayActionAttributeSet::GetAssetCategories() const
{
	static const auto Categories = { FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")), EAssetCategoryPaths::Blueprint };
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayActionAttributeSet::GetClassTypeActions(
	const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayActionAttributeSet::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
    UGameplayActionAttributeSetFactory* Factory = NewObject<UGameplayActionAttributeSetFactory>();

    if (InBlueprint)
    {
        Factory->ParentClass = InBlueprint->GeneratedClass;
    }

    return Factory;
}


namespace MenuExtension_GameplayAttributeSet
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

            const UAssetDefinition_GameplayActionAttributeSet* AssetDefinition = Cast<UAssetDefinition_GameplayActionAttributeSet>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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

            UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayActionAttributeSetBlueprint::StaticClass());

            FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
            Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
            {
                if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
                {
                    if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayActionAttributeSetBlueprint::StaticClass()))
                    {
                        const TAttribute<FText> Label = LOCTEXT("AttributeSet_NewDerivedBlueprint", "Create Child Attribute Set");
                        const TAttribute<FText> ToolTip = LOCTEXT("AttributeSet_NewDerivedBlueprintTooltip", "Creates a Child Attribute Set Blueprint based on the current one.");
                        const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint");

                        FToolUIAction DeriveNewBlueprint;
                        DeriveNewBlueprint.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewDerivedBlueprint, *SelectedAssetPtr);

                        InSection.AddMenuEntry("CreateChildAttributeSet", Label, ToolTip, Icon, DeriveNewBlueprint);
                    }
                }
            }));
        }));
    });
}

#undef LOCTEXT_NAMESPACE
