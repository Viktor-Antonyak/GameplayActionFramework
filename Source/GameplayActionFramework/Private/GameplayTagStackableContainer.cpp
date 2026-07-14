// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayTagStackableContainer.h"


void FGameplayTagStackableContainer::AddTag(const FGameplayTag& Tag, int32 Quantity)
{
	if (!Tag.IsValid() || Quantity <= 0)
	{
		return;
	}

	// Explicit count - only the leaf tag itself
	int32& ExplicitCount = ExplicitTagCounts.FindOrAdd(Tag);
	
	ExplicitCount += Quantity;

	// Aggregated count - leaf + all parents
	const FGameplayTagContainer TagAndParents = Tag.GetGameplayTagParents();
	
	for (const FGameplayTag& TagOrParent : TagAndParents)
	{
		int32& Count = TagContainer.FindOrAdd(TagOrParent);
		
		Count += Quantity;
	}
}

void FGameplayTagStackableContainer::AddTags(const FGameplayTagContainer& Tags, int32 Quantity)
{
	for (const FGameplayTag& Tag : Tags)
	{
		AddTag(Tag, Quantity);
	}
}

void FGameplayTagStackableContainer::RemoveTag(const FGameplayTag& Tag, int32 Quantity)
{
	if (!Tag.IsValid() || Quantity <= 0) { return; }

	int32* ExplicitCount = ExplicitTagCounts.Find(Tag);
	if (!ensureMsgf(ExplicitCount, TEXT("RemoveTag called for tag %s with no matching Add"), *Tag.ToString()))
	{
		return;
	}

	// Clamp so we never remove more than this tag actually contributed.
	const int32 ActualQuantity = FMath::Min(Quantity, *ExplicitCount);

	*ExplicitCount -= ActualQuantity;
	if (*ExplicitCount <= 0)
	{
		ExplicitTagCounts.Remove(Tag);
	}

	const FGameplayTagContainer TagAndParents = Tag.GetGameplayTagParents();
	for (const FGameplayTag& TagOrParent : TagAndParents)
	{
		int32* Count = TagContainer.Find(TagOrParent);
		if (Count)
		{
			*Count -= ActualQuantity;  
			if (*Count <= 0)
			{
				TagContainer.Remove(TagOrParent);
			}
		}
	}
}

void FGameplayTagStackableContainer::RemoveTags(const FGameplayTagContainer& Tags, int32 Quantity)
{
	for (const FGameplayTag& Tag : Tags)
	{
		RemoveTag(Tag, Quantity);
	}
}

FGameplayTagContainer FGameplayTagStackableContainer::GetExplicitGameplayTags() const
{
	FGameplayTagContainer Result;
	
	for (const auto& Pair : ExplicitTagCounts)
	{
		if (Pair.Value > 0)
		{
			Result.AddTagFast(Pair.Key); 
		}
	}
	return Result;
}

FGameplayTagContainer FGameplayTagStackableContainer::GetAggregatedGameplayTags() const
{
	FGameplayTagContainer Result;
	
	for (const auto& Pair : TagContainer)
	{
		if (Pair.Value > 0)
		{
			Result.AddTagFast(Pair.Key);
		}
	}
	return Result;
}

FString FGameplayTagStackableContainer::ToStringExplicit() const
{
	return TagCountMapToString(ExplicitTagCounts);
}

FString FGameplayTagStackableContainer::ToStringAggregated() const
{
	return TagCountMapToString(TagContainer);
}

FString FGameplayTagStackableContainer::TagCountMapToString(const TMap<FGameplayTag, int32>& Map)
{
	if (Map.Num() == 0)
	{
		return TEXT("None");
	}

	TArray<FString> Entries;
	Entries.Reserve(Map.Num());

	for (const auto& Pair : Map)
	{
		Entries.Add(FString::Printf(TEXT("%s (x%d)"), *Pair.Key.ToString(), Pair.Value));
	}

	Entries.Sort();

	return FString::Join(Entries, TEXT(", "));
}
