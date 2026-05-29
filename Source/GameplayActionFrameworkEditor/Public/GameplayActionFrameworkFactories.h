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
class UGameplayAttributeSetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayAttributeSetFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }
	
	TSubclassOf<class UGameplayAttributeSet> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Effects Blueprints */
UCLASS()
class UGameplayEffectFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayEffectFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }

	TSubclassOf<class UGameplayEffect> ParentClass = nullptr;
};

/** Factory for creating new Gameplay Effect Magnitude Calculation Blueprints */
UCLASS()
class UGameplayEffectMagnitudeCalculationFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameplayEffectMagnitudeCalculationFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override { return true; }

	TSubclassOf<class UGameplayEffectMagnitudeCalculation> ParentClass = nullptr;
};

