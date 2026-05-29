// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Script/AssetDefinition_Blueprint.h"
#include "AssetDefinition_GameplayEffect.generated.h"

UCLASS()
class UAssetDefinition_GameplayEffect : public UAssetDefinition_Blueprint
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual TWeakPtr<IClassTypeActions> GetClassTypeActions(const FAssetData& AssetData) const override;
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;
};
