// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayAttributeSet.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "GameplayActionFrameworkFactories.h"
#include "Blueprints/GameplayAttributeSetBlueprint.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayAttributeSet"

FText UAssetDefinition_GameplayAttributeSet::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Attribute Set"));
}

FLinearColor UAssetDefinition_GameplayAttributeSet::GetAssetColor() const
{
	return FLinearColor(1.0f, 0.0f, 0.4f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayAttributeSet::GetAssetClass() const
{
	return UGameplayAttributeSetBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayAttributeSet::GetAssetCategories() const
{
	static const auto Categories = { FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")), EAssetCategoryPaths::Blueprint };
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayAttributeSet::GetClassTypeActions(
	const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayAttributeSet::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
    UGameplayAttributeSetFactory* Factory = NewObject<UGameplayAttributeSetFactory>();

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

            const UAssetDefinition_GameplayAttributeSet* AssetDefinition = Cast<UAssetDefinition_GameplayAttributeSet>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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

            UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayAttributeSetBlueprint::StaticClass());

            FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
            Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
            {
                if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
                {
                    if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayAttributeSetBlueprint::StaticClass()))
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
