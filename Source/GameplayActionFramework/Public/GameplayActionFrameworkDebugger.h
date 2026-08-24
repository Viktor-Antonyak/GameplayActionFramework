// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#if WITH_GAMEPLAY_DEBUGGER
#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"

class APlayerController;
class AActor;
class UGameplayActionComponent;

class FGAFGameplayDebuggerCategory : public FGameplayDebuggerCategory
{
public:

	FGAFGameplayDebuggerCategory();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

private:

	/** Draws a header followed by items laid out in a grid (MaxRows rows per column, wrapping horizontally). */
	void DrawGridSection(FGameplayDebuggerCanvasContext& CanvasContext, const FString& Header, const TArray<TPair<FString, FColor>>& Items, float& CursorY, const float StartX, const int32 MaxRows);

	struct FActionData
	{
		FString ActionName;
		bool bIsActive = false;
	};

	struct FActiveEffectData
	{
		FString EffectName;
		int32 StackCount = 0;
		float RemainingDuration = -1.f;
		bool bInhibited = false;
	};

	struct FAttributeData
	{
		FString AttributeName;
		float BaseValue = 0.f;
		float CurrentValue = 0.f;
	};

	struct FAttributeSetData
	{
		FString SetName;
		TArray<FAttributeData> Attributes;
	};

	struct FRepData
	{
		FString ActorName;
		TArray<FActionData> OwnedActions;
		TArray<FString> OwnedTriggeredActions;
		TArray<FActiveEffectData> ActiveEffects;
		FString OwnedGameplayTags;
		TArray<FAttributeSetData> AttributeSets;
	};

	FRepData DataPack;
};
#endif

