// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GameplayActionFrameworkFactories.generated.h"

/** Factory for creating new Gameplay Action Blueprints */
UCLASS()
class UGameplayActionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayActionFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }
	
	TSubclassOf<class UGameplayAction> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Attribute Set Blueprints */
UCLASS()
class UGameplayActionAttributeSetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayActionAttributeSetFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }
	
	TSubclassOf<class UGameplayActionAttributeSet> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Effects Blueprints */
UCLASS()
class UGameplayActionEffectFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayActionEffectFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }

	TSubclassOf<class UGameplayActionEffect> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Effect Magnitude Calculation Blueprints */
UCLASS()
class UGameplayActionEffectMagnitudeCalculationFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayActionEffectMagnitudeCalculationFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }

	TSubclassOf<class UGameplayActionEffectMagnitudeCalc> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Effect Execution Calculation Blueprints */
UCLASS()
class UGameplayActionEffectExecutionCalculationFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayActionEffectExecutionCalculationFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }

	TSubclassOf<class UGameplayActionEffectExecutionCalc> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Action Fragment Blueprints */
UCLASS()
class UGameplayActionFragmentFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayActionFragmentFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }

	TSubclassOf<class UGameplayActionFragment> ParentClass = nullptr;
};

