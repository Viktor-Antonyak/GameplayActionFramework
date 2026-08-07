// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GameplayAttributeSet.h"
#include "GameplayEffect.generated.h"

class UGameplayEffectMagnitudeCalculation;
class UGameplayEffectExecutionCalculation;

/**
 * Gameplay Effect duration types.
 */
UENUM(BlueprintType)
enum class EGameplayEffectDuration : uint8
{
    Instant,
    Infinite,
    HasDuration
};


UENUM(BlueprintType)
enum class EGameplayEffectStacking : uint8
{
    None,
    AggregateBySource,
    AggregateByTarget
};

USTRUCT(BlueprintType)
struct FGameplayModifierInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    TSubclassOf<UGameplayEffectMagnitudeCalculation> MagnitudeCalculation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    FGameplayAttribute Attribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
    EGameplayModOp ModifierOp = EGameplayModOp::Additive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
    float Magnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
    FGameplayTag DataTag;
};

USTRUCT(BlueprintType)
struct FGameplayEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    const class UGameplayEffect* Effect = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, float> SetByCallerMagnitudes;
    
    UPROPERTY(BlueprintReadWrite)
    int32 Level = 1;
    
    /** The actor that caused/instigated this effect (e.g. the attacker). Used for AggregateBySource stacking
     * and available to MagnitudeCalculation/ExecutionCalculation for source-relative logic. 
     */
    UPROPERTY(BlueprintReadWrite)
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
struct FActiveGameplayEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FGameplayEffectSpec Spec;

    UPROPERTY(BlueprintReadOnly)
    float StartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Duration = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FActiveGameplayEffectHandle Handle;

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
class GAMEPLAYACTIONFRAMEWORK_API UGameplayEffect : public UDataAsset
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration")
    EGameplayEffectDuration DurationPolicy = EGameplayEffectDuration::Instant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Duration", meta=(EditCondition="DurationPolicy == EGameplayEffectDuration::HasDuration"))
    float DurationMagnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    TSubclassOf<UGameplayEffectExecutionCalculation> ExecutionCalculation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
    TArray<FGameplayModifierInfo> Modifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer GrantedTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags | Requirements")
    FGameplayTagContainer ApplicationRequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags | Requirements")
    FGameplayTagContainer ApplicationBlockedTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Period", meta=(EditCondition="DurationPolicy != EGameplayEffectDuration::Instant"))
    float Period = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Period", meta=(EditCondition="Period > 0.0f"))
    bool bExecutePeriodicOnApplication = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stacking")
    EGameplayEffectStacking StackingPolicy = EGameplayEffectStacking::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stacking", meta=(EditCondition="StackingPolicy != EGameplayEffectStacking::None"))
    int32 StackLimitCount = 1;
};
