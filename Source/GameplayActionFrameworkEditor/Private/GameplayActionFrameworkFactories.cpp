// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionFrameworkFactories.h"
#include "GameplayAction.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayActionEffect.h"
#include "GameplayActionEffectMagnitudeCalc.h"
#include "Blueprints/GameplayActionBlueprint.h"
#include "Blueprints/GameplayActionAttributeSetBlueprint.h"
#include "Blueprints/GameplayActionEffectBlueprint.h"
#include "Blueprints/GameplayActionEffectMagnitudeCalcBlueprint.h"
#include "Blueprints/GameplayActionEffectExecutionCalcBlueprint.h"
#include "GameplayActionEffectExecutionCalc.h"
#include "GameplayActionFragment.h"
#include "Blueprints/GameplayActionFragmentBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "Editor/ClassViewer/Private/UnloadedBlueprintData.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Templates/Casts.h"

// Gameplay Action class Filter
class FGameplayActionClassFilter : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildOfClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UGameplayActionFactory::UGameplayActionFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayActionBlueprint::StaticClass();
}

UObject* UGameplayActionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayAction::StaticClass();

	UGameplayActionBlueprint* NewBP = CastChecked<UGameplayActionBlueprint>(FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayActionBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()));

	if (NewBP)
	{
		UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(NewBP);
		if (EventGraph)
		{
			int32 NodeInstance = 0;

			// Add OnExecuteAction
			FKismetEditorUtilities::AddDefaultEventNode(NewBP, EventGraph, GET_FUNCTION_NAME_CHECKED(UGameplayAction, OnExecuteAction), UGameplayAction::StaticClass(), NodeInstance);

			// Add OnEndAction
			NodeInstance += 150;
			FKismetEditorUtilities::AddDefaultEventNode(NewBP, EventGraph, GET_FUNCTION_NAME_CHECKED(UGameplayAction, OnEndAction), UGameplayAction::StaticClass(), NodeInstance);
		}
	}

	return NewBP;
}

bool UGameplayActionFactory::ConfigureProperties()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bExpandAllNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	
	TSharedPtr<FGameplayActionClassFilter> Filter = MakeShared<FGameplayActionClassFilter>();
	Filter->AllowedChildOfClasses.Add(UGameplayAction::StaticClass());
	
	Options.ClassFilters.Add(Filter.ToSharedRef());
	
	const FText TitleText = NSLOCTEXT("GameplayActionFramework", "CreateGameplayActionOptions", "Pick Parent Class for New Gameplay Action");
	
	UClass* ChosenClass = nullptr;
	
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayAction::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

// Gameplay Action Attribute Set class Filter
class FGameplayActionAttributeSetClassFilter : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildOfClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UGameplayActionAttributeSetFactory::UGameplayActionAttributeSetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayActionAttributeSetBlueprint::StaticClass();
}

UObject* UGameplayActionAttributeSetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayActionAttributeSet::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayActionAttributeSetBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UGameplayActionAttributeSetFactory::ConfigureProperties()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bExpandAllNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	
	TSharedPtr<FGameplayActionAttributeSetClassFilter> Filter = MakeShared<FGameplayActionAttributeSetClassFilter>();
	Filter->AllowedChildOfClasses.Add(UGameplayActionAttributeSet::StaticClass());
	
	Options.ClassFilters.Add(Filter.ToSharedRef());
	
	const FText TitleText = NSLOCTEXT("GameplayActionFramework", "CreateGameplayAttributeSetOptions", "Pick Parent Class for New Attribute Set");
	
	UClass* ChosenClass = nullptr;
	
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayActionAttributeSet::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

// Gameplay Action Effect class Filter
class FGameplayActionEffectClassFilter : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildOfClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UGameplayActionEffectFactory::UGameplayActionEffectFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayActionEffectBlueprint::StaticClass();
}

UObject* UGameplayActionEffectFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayActionEffect::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayActionEffectBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UGameplayActionEffectFactory::ConfigureProperties()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bExpandAllNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	
	TSharedPtr<FGameplayActionEffectClassFilter> Filter = MakeShared<FGameplayActionEffectClassFilter>();
	Filter->AllowedChildOfClasses.Add(UGameplayActionEffect::StaticClass());
	
	Options.ClassFilters.Add(Filter.ToSharedRef());
	
	const FText TitleText = NSLOCTEXT("GameplayActionFramework", "CreateGameplayActionEffectOptions", "Pick Parent Class for New Gameplay Effect");
	
	UClass* ChosenClass = nullptr;
	
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayActionEffect::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

// Gameplay Action Effect Mag Calc class Filter
class FGameplayActionEffectMagCalcClassFilter : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildOfClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UGameplayActionEffectMagnitudeCalculationFactory::UGameplayActionEffectMagnitudeCalculationFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayActionEffectMagnitudeCalcBlueprint::StaticClass();
}

UObject* UGameplayActionEffectMagnitudeCalculationFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayActionEffectMagnitudeCalc::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayActionEffectMagnitudeCalcBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UGameplayActionEffectMagnitudeCalculationFactory::ConfigureProperties()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bExpandAllNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	
	TSharedPtr<FGameplayActionEffectMagCalcClassFilter> Filter = MakeShared<FGameplayActionEffectMagCalcClassFilter>();
	Filter->AllowedChildOfClasses.Add(UGameplayActionEffectMagnitudeCalc::StaticClass());
	
	Options.ClassFilters.Add(Filter.ToSharedRef());
	
	const FText TitleText = NSLOCTEXT("GameplayActionFramework", "CreateGameplayEffectMagnitudeCalculationOptions", "Pick Parent Class for New Gameplay Effect Magnitude Calculation");
	
	UClass* ChosenClass = nullptr;
	
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayActionEffectMagnitudeCalc::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

// Gameplay Action Effect Exec Calc class Filter
class FGameplayActionEffectExecCalcClassFilter : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildOfClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UGameplayActionEffectExecutionCalculationFactory::UGameplayActionEffectExecutionCalculationFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayActionEffectExecutionCalcBlueprint::StaticClass();
}

UObject* UGameplayActionEffectExecutionCalculationFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayActionEffectExecutionCalc::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayActionEffectExecutionCalcBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UGameplayActionEffectExecutionCalculationFactory::ConfigureProperties()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bExpandAllNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	
	TSharedPtr<FGameplayActionEffectExecCalcClassFilter> Filter = MakeShared<FGameplayActionEffectExecCalcClassFilter>();
	Filter->AllowedChildOfClasses.Add(UGameplayActionEffectExecutionCalc::StaticClass());
	
	Options.ClassFilters.Add(Filter.ToSharedRef());
	
	const FText TitleText = NSLOCTEXT("GameplayActionFramework", "CreateGameplayActionEffectExecutionCalculationOptions", "Pick Parent Class for New Gameplay Effect Execution Calculation");
	
	UClass* ChosenClass = nullptr;
	
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayActionEffectExecutionCalc::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

// Gameplay Action Fragment class Filter
class FGameplayActionFragmentClassFilter : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildOfClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InFilterFuncs->IfInChildOfClassesSet(AllowedChildOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UGameplayActionFragmentFactory::UGameplayActionFragmentFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayActionFragmentBlueprint::StaticClass();
}

UObject* UGameplayActionFragmentFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayActionFragment::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayActionFragmentBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UGameplayActionFragmentFactory::ConfigureProperties()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bExpandAllNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	
	TSharedPtr<FGameplayActionFragmentClassFilter> Filter = MakeShared<FGameplayActionFragmentClassFilter>();
	Filter->AllowedChildOfClasses.Add(UGameplayActionFragment::StaticClass());
	
	Options.ClassFilters.Add(Filter.ToSharedRef());
	
	const FText TitleText = NSLOCTEXT("GameplayActionFramework", "CreateGameplayActionFragmentOptions", "Pick Parent Class for New Gameplay Action Fragment");
	
	UClass* ChosenClass = nullptr;
	
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayActionFragment::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

