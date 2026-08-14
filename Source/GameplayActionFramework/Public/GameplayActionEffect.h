// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayActionEffectTypes.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayActionEffect.generated.h"

class UGameplayActionEffectMagnitudeCalc;
class UGameplayActionEffectExecutionCalc;

/**
 * Gameplay Effect duration types.
 */
UENUM(BlueprintType)
enum class EGameplayActionEffectDuration : uint8
{
    Instant,
    Infinite,
    HasDuration
};


UENUM(BlueprintType)
enum class EGameplayActionEffectStacking : uint8
{
    None,
    AggregateBySource,
    AggregateByTarget
};

USTRUCT(BlueprintType)
struct FGameplayActionEffectModifierInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    TSubclassOf<UGameplayActionEffectMagnitudeCalc> MagnitudeCalculation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    FGameplayActionAttribute Attribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
    EGameplayActionEffectModOp ModifierOp = EGameplayActionEffectModOp::Additive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
    float Magnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
    FGameplayTag DataTag;
};

USTRUCT(BlueprintType)
struct FGameplayActionEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Gameplay Action Effect")
    const UGameplayActionEffect* Effect = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Gameplay Action Effect")
    TMap<FGameplayTag, float> SetByCallerMagnitudes;
    
    UPROPERTY(BlueprintReadWrite, Category = "Gameplay Action Effect")
    int32 Level = 1;
    
    /** The actor that caused/instigated this effect (e.g. the attacker). Used for AggregateBySource stacking
     * and available to MagnitudeCalculation/ExecutionCalculation for source-relative logic. 
     */
    UPROPERTY(BlueprintReadWrite, Category = "Gameplay Action Effect")
    TWeakObjectPtr<AActor> SourceActor;

    void SetSetByCallerMagnitude(FGameplayTag Tag, float Magnitude)
    {
        if (Tag.IsValid())
        {
            SetByCallerMagnitudes.FindOrAdd(Tag) = Magnitude;
        }
    }

    float GetSetByCallerMagnitude(FGameplayTag Tag) const
    {
        return SetByCallerMagnitudes.FindRef(Tag);
    }
};

USTRUCT(BlueprintType)
struct FActiveGameplayActionEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Action Effect")
    FGameplayActionEffectSpec Spec;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Action Effect")
    float StartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Action Effect")
    float Duration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Action Effect")
    FActiveGameplayActionEffectHandle Handle;

    FTimerHandle DurationHandle;

    FTimerHandle PeriodTimerHandle;

    int32 StackCount = 1;

    /** 
     * Whether this effect's modifiers are currently suppressed via UGameplayActionComponent::SetEffectInhibited.
     * Applies to the entire active effect (all stacks at once) — see SetEffectInhibited for details.
     */
    bool bIsInhibited = false;
};

/**
 * Base Gameplay Effect to modify attribute/grant Gameplay Tags
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionEffect : public UDataAsset
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
    EGameplayActionEffectDuration DurationPolicy = EGameplayActionEffectDuration::Instant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration", meta=(EditCondition="DurationPolicy == EGameplayActionEffectDuration::HasDuration"))
    float DurationMagnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    TSubclassOf<UGameplayActionEffectExecutionCalc> ExecutionCalculation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
    TArray<FGameplayActionEffectModifierInfo> Modifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer GrantedTags;

    /** Tags the target must own for the effect to apply; empty means no requirement. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags | Requirements")
    FGameplayTagContainer ApplicationRequiredTags;

    /** Tags that, if present on the target, prevent the effect from applying. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags | Requirements")
    FGameplayTagContainer ApplicationBlockedTags;

    /** Delay in seconds between periodic executions; 0 disables periodic ticking. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Period", meta=(EditCondition="DurationPolicy != EGameplayActionEffectDuration::Instant"))
    float Period = 0.0f;

    /** If true, the first periodic execution fires immediately on application. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Period", meta=(EditCondition="Period > 0.0f"))
    bool bExecutePeriodicOnApplication = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stacking")
    EGameplayActionEffectStacking StackingPolicy = EGameplayActionEffectStacking::None;

    /** Maximum number of concurrent stacks from the same source. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stacking", meta=(EditCondition="StackingPolicy != EGameplayActionEffectStacking::None"))
    int32 StackLimitCount = 1;
};
