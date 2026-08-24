// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionFrameworkDebugger.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "GameplayActionInterface.h"
#include "GameplayActionComponent.h"
#include "GameplayAction.h"
#include "GameplayActionEffect.h"
#include "GameplayActionAttributeSet.h"

FGAFGameplayDebuggerCategory::FGAFGameplayDebuggerCategory()
{
	bShowOnlyWithDebugActor = true;
}

TSharedRef<FGameplayDebuggerCategory> FGAFGameplayDebuggerCategory::MakeInstance()
{
	return MakeShareable(new FGAFGameplayDebuggerCategory);
}

namespace
{
	FString GetCleanClassName(const UObject* Object)
	{
		FString Name = Object ? Object->GetName() : TEXT("Invalid");
		Name.RemoveFromStart(TEXT("Default__"));
		Name.RemoveFromEnd(TEXT("_C"));
		return MoveTemp(Name);
	}
}

void FGAFGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!DebugActor)
	{
		return;
	}

	UGameplayActionComponent* GAC = DebugActor->FindComponentByClass<UGameplayActionComponent>();
	if (!GAC && DebugActor->Implements<UGameplayActionInterface>())
	{
		GAC = IGameplayActionInterface::Execute_GetGameplayActionComponent(DebugActor);
	}

	if (!GAC)
	{
		return;
	}

	DataPack = FRepData();
	DataPack.ActorName = DebugActor->GetName();

	for (const FGameplayActionSpec& Spec : GAC->GetAddedActions())
	{
		if (Spec.IsValid())
		{
			FActionData& ActionData = DataPack.OwnedActions.AddDefaulted_GetRef();
			ActionData.ActionName = GetCleanClassName(Spec.Action->GetClass());
			ActionData.bIsActive = Spec.Action->IsActive();
		}
	}

	for (const UGameplayAction* Action : GAC->GetActiveTriggeredActions())
	{
		if (IsValid(Action))
		{
			DataPack.OwnedTriggeredActions.Add(GetCleanClassName(Action->GetClass()));
		}
	}

	const float CurrentTime = GAC->GetWorld() ? GAC->GetWorld()->GetTimeSeconds() : 0.f;
	for (const TTuple<FActiveGameplayActionEffectHandle, FActiveGameplayActionEffect>& Pair : GAC->GetActiveEffects().GetAll())
	{
		const FActiveGameplayActionEffect& Effect = Pair.Value;
		if (!Effect.Spec.Effect)
		{
			continue;
		}

		FActiveEffectData& EffectData = DataPack.ActiveEffects.AddDefaulted_GetRef();
		EffectData.EffectName = GetCleanClassName(Effect.Spec.Effect);
		EffectData.StackCount = Effect.StackCount;
		EffectData.RemainingDuration = Effect.Duration > 0.f ? FMath::Max(0.f, Effect.Duration - (CurrentTime - Effect.StartTime)) : -1.f;
		EffectData.bInhibited = Effect.bIsInhibited;
	}

	const FGameplayTagContainer OwnedTags = GAC->GetOwnedGameplayTags();

	TArray<FString> TagNames;
	for (int32 TagIndex = 0; TagIndex < OwnedTags.Num(); ++TagIndex)
	{
		const FGameplayTag Tag = OwnedTags.GetByIndex(TagIndex);

		bool bIsParentTag = false;
		for (int32 OtherIndex = 0; OtherIndex < OwnedTags.Num(); ++OtherIndex)
		{
			if (OtherIndex != TagIndex && OwnedTags.GetByIndex(OtherIndex).MatchesTag(Tag))
			{
				bIsParentTag = true;
				break;
			}
		}

		if (!bIsParentTag)
		{
			TagNames.Add(FString::Printf(TEXT("%s (x%d)"), *Tag.ToString(), GAC->GetOwnedGameplayTagCount(Tag)));
		}
	}
	TagNames.Sort();
	DataPack.OwnedGameplayTags = FString::Join(TagNames, TEXT(", "));

	for (const UGameplayActionAttributeSet* Set : GAC->GetAttributeSets())
	{
		if (!IsValid(Set))
		{
			continue;
		}

		FAttributeSetData& SetData = DataPack.AttributeSets.AddDefaulted_GetRef();
		SetData.SetName = Set->GetClass()->GetName();

		for (TFieldIterator<FProperty> It(Set->GetClass()); It; ++It)
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(*It);
			if (!StructProp || StructProp->Struct != FGameplayActionAttributeData::StaticStruct())
			{
				continue;
			}

			const FGameplayActionAttributeData* Data = StructProp->ContainerPtrToValuePtr<FGameplayActionAttributeData>(const_cast<UGameplayActionAttributeSet*>(Set));
			if (!Data)
			{
				continue;
			}

			FAttributeData& AttrData = SetData.Attributes.AddDefaulted_GetRef();
			AttrData.AttributeName = It->GetName();
			AttrData.BaseValue = Data->GetBaseValue();
			AttrData.CurrentValue = Data->GetCurrentValue();
		}
	}
}

void FGAFGameplayDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	const float LineHeight = CanvasContext.GetLineHeight();
	constexpr int32 MaxRows = 4;
	const float StartX = CanvasContext.CursorX;
	const float StartY = CanvasContext.CursorY;

	struct FSection
	{
		FString Header;
		TArray<TPair<FString, FColor>> Items;
	};

	TArray<FSection> Sections;

	FSection& GrantSection = Sections.AddDefaulted_GetRef();
	GrantSection.Header = TEXT("Grant Actions");
	for (const FActionData& ActionData : DataPack.OwnedActions)
	{
		GrantSection.Items.Add({ ActionData.ActionName, ActionData.bIsActive ? FColor::Green : FColor::White });
	}

	FSection& TriggeredSection = Sections.AddDefaulted_GetRef();
	TriggeredSection.Header = TEXT("Triggered Actions");
	for (const FString& ActionName : DataPack.OwnedTriggeredActions)
	{
		TriggeredSection.Items.Add({ ActionName, FColor::Green });
	}

	FSection& EffectsSection = Sections.AddDefaulted_GetRef();
	EffectsSection.Header = TEXT("Active Effects");
	for (const FActiveEffectData& EffectData : DataPack.ActiveEffects)
	{
		FString Info = FString::Printf(TEXT("%s [%d]"), *EffectData.EffectName, EffectData.StackCount);
		if (EffectData.RemainingDuration >= 0.f)
		{
			Info += FString::Printf(TEXT(" (%.1fs)"), EffectData.RemainingDuration);
		}
		else
		{
			Info += TEXT(" (Infinite)");
		}

		if (EffectData.bInhibited)
		{
			Info += TEXT(" [INHIBITED]");
		}

		EffectsSection.Items.Add({ Info, EffectData.bInhibited ? FColor::Silver : FColor::White });
	}

	FSection& TagsSection = Sections.AddDefaulted_GetRef();
	TagsSection.Header = TEXT("Owned Tags");
	if (!DataPack.OwnedGameplayTags.IsEmpty())
	{
		TagsSection.Items.Add({ DataPack.OwnedGameplayTags, FColor::White });
	}

	for (const FAttributeSetData& SetData : DataPack.AttributeSets)
	{
		FSection& AttributeSetSection = Sections.AddDefaulted_GetRef();
		AttributeSetSection.Header = SetData.SetName;
		for (const FAttributeData& AttrData : SetData.Attributes)
		{
			AttributeSetSection.Items.Add({ FString::Printf(TEXT("%s: %.1f/%.1f"), *AttrData.AttributeName, AttrData.BaseValue, AttrData.CurrentValue), FColor::White });
		}
	}

	float ContentHeight = LineHeight;
	for (const FSection& Section : Sections)
	{
		ContentHeight += LineHeight * (1 + FMath::Min(Section.Items.Num(), MaxRows));
	}

	if (UCanvas* Canvas = CanvasContext.Canvas.Get())
	{
		constexpr float PadY = 4.f;
		FCanvasTileItem BackgroundTile(
			FVector2D(0.f, StartY - PadY),
			FVector2D(Canvas->SizeX, ContentHeight + PadY * 2.f),
			FLinearColor(0.f, 0.f, 0.f, 0.2f)
		);
		BackgroundTile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(BackgroundTile);
	}

	float CursorY = StartY;
	CanvasContext.PrintfAt(StartX, CursorY, TEXT("--- %s ---"), *DataPack.ActorName);
	CursorY += LineHeight;
	for (const FSection& Section : Sections)
	{
		DrawGridSection(CanvasContext, Section.Header, Section.Items, CursorY, StartX, MaxRows);
	}

	CanvasContext.CursorX = StartX;
	CanvasContext.CursorY = CursorY;
}

void FGAFGameplayDebuggerCategory::DrawGridSection(FGameplayDebuggerCanvasContext& CanvasContext, const FString& Header, const TArray<TPair<FString, FColor>>& Items, float& CursorY, const float StartX, const int32 MaxRows)
{
	const float LineHeight = CanvasContext.GetLineHeight();

	CanvasContext.PrintfAt(StartX, CursorY, TEXT("--- %s ---"), *Header);
	CursorY += LineHeight;

	if (Items.Num() == 0)
	{
		return;
	}

	constexpr float CellPadding = 16.f;
	constexpr float Indent = 12.f;

	float MaxWidth = 0.f;
	for (const TPair<FString, FColor>& Item : Items)
	{
		float Width, Height;
		CanvasContext.MeasureString(Item.Key, Width, Height);
		MaxWidth = FMath::Max(MaxWidth, Width);
	}

	const float CellWidth = MaxWidth + CellPadding;
	const float TopY = CursorY;

	int32 Row = 0;
	int32 Col = 0;
	for (const TPair<FString, FColor>& Item : Items)
	{
		const float X = StartX + Indent + Col * CellWidth;
		const float Y = TopY + Row * LineHeight;
		CanvasContext.PrintfAt(X, Y, Item.Value, TEXT("%s"), *Item.Key);

		Row++;
		if (Row >= MaxRows)
		{
			Row = 0;
			Col++;
		}
	}

	CursorY = TopY + FMath::Min(Items.Num(), MaxRows) * LineHeight;
}
#endif

