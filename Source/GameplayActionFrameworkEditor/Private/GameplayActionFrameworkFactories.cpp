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
#include "Templates/Casts.h"

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

