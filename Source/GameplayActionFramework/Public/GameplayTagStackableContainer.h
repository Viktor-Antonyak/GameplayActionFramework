// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagStackableContainer.generated.h"

USTRUCT()
struct FGameplayTagStackableContainer
{
	GENERATED_BODY()
	
public:
	
	bool HasMatchingGameplayTag(const FGameplayTag& Tag) const
	{
		return TagContainer.FindRef(Tag) > 0;
	}
	
	bool HasAllMatchingTags(const FGameplayTagContainer& Tags) const
	{
		if (!Tags.IsValid())
		{
			return true;
		}
		
		bool bHasAll = true;

		for (const FGameplayTag& Tag : Tags)
		{
			if (TagContainer.FindRef(Tag) <= 0)
			{
				bHasAll = false;
				break;
			}
		}
		
		return bHasAll;
	}
	
	bool HasAnyMatchingTags(const FGameplayTagContainer& Tags) const
	{
		if (!Tags.IsValid())
		{
			return true;
		}
		
		bool bHasAny = false;

		for (const FGameplayTag& Tag : Tags)
		{
			if (TagContainer.FindRef(Tag) > 0)
			{
				bHasAny = true;
				break;
			}
		}
		
		return bHasAny;
	}
	
	int32 GetTagCount(const FGameplayTag& Tag) const
	{
		return TagContainer.FindRef(Tag);
	}
	
	/** Returns true if the container has at least one active tag. */
	bool IsValid() const
	{
		return TagContainer.Num() > 0;
	}
	
	void Empty()
	{
		TagContainer.Empty();
		
		ExplicitTagCounts.Empty();
	}
	
	void AddTag(const FGameplayTag& Tag, int32 Quantity = 1);
	
	void AddTags(const FGameplayTagContainer& Tags, int32 Quantity = 1);
	
	void RemoveTag(const FGameplayTag& Tag, int32 Quantity = 1);
	
	void RemoveTags(const FGameplayTagContainer& Tags, int32 Quantity = 1);
	
	FGameplayTagContainer GetExplicitGameplayTags() const;
	
	FGameplayTagContainer GetAggregatedGameplayTags() const;
	
	/** Returns a human-readable string of explicitly added tags with their stack counts, e.g. "State.Buff.Stun (x2), State.Poisoned (x1)" */
	FString ToStringExplicit() const;

	/** Returns a human-readable string of all active tags (including implicit parents) with their aggregated stack counts. */
	FString ToStringAggregated() const;
	
private:
	
	/** Aggregated counts: leaf tag + all its parent tags. Source of truth for Has* queries. */
	TMap<FGameplayTag, int32> TagContainer;

	/** Explicit counts: only the exact tags passed to AddTag, no parent expansion. Used for debug/inspection only. */
	TMap<FGameplayTag, int32> ExplicitTagCounts;
	
	static FString TagCountMapToString(const TMap<FGameplayTag, int32>& Map);
};
