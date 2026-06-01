// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionFrameworkFactories.h"
#include "GameplayAction.h"
#include "GameplayAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectMagnitudeCalculation.h"
#include "Blueprints/GameplayActionBlueprint.h"
#include "Blueprints/GameplayAttributeSetBlueprint.h"
#include "Blueprints/GameplayEffectBlueprint.h"
#include "Blueprints/GameplayEffectMagnitudeCalculationBlueprint.h"
#include "Blueprints/GameplayEffectExecutionCalculationBlueprint.h"
#include "GameplayEffectExecutionCalculation.h"
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

UGameplayAttributeSetFactory::UGameplayAttributeSetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayAttributeSetBlueprint::StaticClass();
}

UObject* UGameplayAttributeSetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayAttributeSet::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayAttributeSetBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

UGameplayEffectFactory::UGameplayEffectFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayEffectBlueprint::StaticClass();
}

UObject* UGameplayEffectFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayEffect::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayEffectBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

UGameplayEffectMagnitudeCalculationFactory::UGameplayEffectMagnitudeCalculationFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayEffectMagnitudeCalculationBlueprint::StaticClass();
}

UObject* UGameplayEffectMagnitudeCalculationFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayEffectMagnitudeCalculation::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayEffectMagnitudeCalculationBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

UGameplayEffectExecutionCalculationFactory::UGameplayEffectExecutionCalculationFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameplayEffectExecutionCalculationBlueprint::StaticClass();
}

UObject* UGameplayEffectExecutionCalculationFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ValidParent = ParentClass ? ParentClass.Get() : UGameplayEffectExecutionCalculation::StaticClass();

	return FKismetEditorUtilities::CreateBlueprint(ValidParent, InParent, InName, BPTYPE_Normal, UGameplayEffectExecutionCalculationBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

