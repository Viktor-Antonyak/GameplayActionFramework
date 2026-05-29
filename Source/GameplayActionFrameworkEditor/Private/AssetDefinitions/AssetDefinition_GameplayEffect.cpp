// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "AssetDefinitions/AssetDefinition_GameplayEffect.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Blueprints/GameplayEffectBlueprint.h"
#include "GameplayActionFrameworkFactories.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/BlueprintSupport.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_GameplayEffect"

FText UAssetDefinition_GameplayEffect::GetAssetDisplayName() const
{
	return FText::FromString(TEXT("Gameplay Effect"));
}

FLinearColor UAssetDefinition_GameplayEffect::GetAssetColor() const
{
	return FLinearColor(0.2f, 1.0f, 0.2f);
}

TSoftClassPtr<UObject> UAssetDefinition_GameplayEffect::GetAssetClass() const
{
	return UGameplayEffectBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayEffect::GetAssetCategories() const
{
	static const auto Categories = {
		FAssetCategoryPath(NSLOCTEXT("AssetDefinitions", "GAF_Category", "Gameplay Action Framework")),
		EAssetCategoryPaths::Blueprint
	};
	return Categories;
}

TWeakPtr<IClassTypeActions> UAssetDefinition_GameplayEffect::GetClassTypeActions(const FAssetData& AssetData) const
{
	return Super::GetClassTypeActions(AssetData);
}

UFactory* UAssetDefinition_GameplayEffect::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameplayEffectFactory* Factory = NewObject<UGameplayEffectFactory>();

	if (InBlueprint)
	{
		Factory->ParentClass = InBlueprint->GeneratedClass;
	}

	return Factory;
}

//----------------------------------
// Menu Extension
//----------------------------------

namespace MenuExtension_GameplayEffect
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

			const UAssetDefinition_GameplayEffect* AssetDefinition = Cast<UAssetDefinition_GameplayEffect>(UAssetDefinitionRegistry::Get()->GetAssetDefinitionForClass(ParentBlueprint->GetClass()));
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

			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameplayEffectBlueprint::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				if (const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
				{
					if (const FAssetData* SelectedAssetPtr = Context->GetSingleSelectedAssetOfType(UGameplayEffectBlueprint::StaticClass()))
					{
						const TAttribute<FText> Label = LOCTEXT("GameplayEffect_NewDerivedBlueprint", "Create Child Effect");
						const TAttribute<FText> ToolTip = LOCTEXT("GameplayEffect_NewDerivedBlueprintTooltip", "Creates a Child Gameplay Effect Blueprint based on the current one.");
						const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint");

						FToolUIAction DeriveNewBlueprint;
						DeriveNewBlueprint.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewDerivedBlueprint, *SelectedAssetPtr);

						InSection.AddMenuEntry("CreateChildGameplayEffect", Label, ToolTip, Icon, DeriveNewBlueprint);
					}
				}
			}));
		}));
	});
}

#undef LOCTEXT_NAMESPACE
