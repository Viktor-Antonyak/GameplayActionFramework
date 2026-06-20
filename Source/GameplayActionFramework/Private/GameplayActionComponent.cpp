// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionComponent.h"
#include "Engine/TimerHandle.h"
#include "Engine/World.h"
#include "Templates/Tuple.h"
#include "TimerManager.h"
#include "GameplayActionSpec.h"
#include "GameplayAttributeSet.h"
#include "GameplayAction.h" 
#include "GameplayEffect.h"
#include "GameplayEffectMagnitudeCalculation.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/MovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayEffect.h"
#include "UObject/Object.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayActionComponent)

DEFINE_LOG_CATEGORY(LogGameplayActionComponent);

UGameplayActionComponent::UGameplayActionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGameplayActionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitActorInfo();
	InitAttributes();
	InitActions();
}

void UGameplayActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GameplayActionActorInfo.Reset();
	GameplayTagContainer.Reset();

	for (UGameplayAction* ActiveAction : ActiveActions)
	{
		if (IsValid(ActiveAction))
		{
			ActiveAction->RequestEndAction();
		}
	}

	for (UGameplayAction* TriggeredAction : ActiveTriggeredActions)
	{
		if (IsValid(TriggeredAction))
		{
			TriggeredAction->RequestEndAction();
		}
	}
	
	for (const FGameplayActionSpec& AddedAction : AddedActions)
	{
		if (IsValid(AddedAction.Action))
		{
			AddedAction.Action->MarkAsGarbage();
		}
	}
	
	ActiveActions.Empty();
	ActiveTriggeredActions.Empty();
	AddedActions.Empty();
	AttributeSets.Empty(); 
}

// --- Action Management ---
FGameplayActionSpec UGameplayActionComponent::AddGameplayAction(const TSubclassOf<UGameplayAction> Action, const int32 Level)
{
	if (!IsValid(Action) || !GameplayActionActorInfo.IsValid())
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("AddGameplayAction: Invalid Action Class"));
		return FGameplayActionSpec();
	}
	
	const FGameplayActionSpec* TestAction = AddedActions.FindByPredicate([Action](const FGameplayActionSpec& Spec)
	{
		return Spec.Action && Spec.Action->GetClass() == Action;
	});

	if (TestAction && TestAction->IsValid())
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("AddGameplayAction: Action class %s is already added."), *Action->GetName());
		return FGameplayActionSpec();
	}

	const UGameplayAction* DefaultAction = Action->GetDefaultObject<UGameplayAction>();
	if (DefaultAction && DefaultAction->ActionType == EGameplayActionType::Triggered)
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("AddGameplayAction: Action class %s is a Triggered type and cannot be added as a permanent action."), *Action->GetName());
		return FGameplayActionSpec();
	}
	
	UGameplayAction* NewAction = NewObject<UGameplayAction>(this, Action);
	
	FGameplayActionSpec NewActionSpec;
	NewActionSpec.Action = NewAction;
	NewActionSpec.Level = Level;
	NewActionSpec.InputID = NewAction->InputID;
	
	AddedActions.Add(NewActionSpec);
	
	NewAction->InitializeAction(GameplayActionActorInfo, Level);
	
	return NewActionSpec;
}

void UGameplayActionComponent::RemoveGameplayAction(const FGameplayActionSpec& ActionSpec)
{
	AddedActions.Remove(ActionSpec);

	if (ActiveActions.Contains(ActionSpec.Action))
	{
		ActionSpec.Action->RequestEndAction();
	}

	if (IsValid(ActionSpec.Action)) 
	{
	    ActionSpec.Action->DeinitializeAction();
		ActionSpec.Action->MarkAsGarbage();
	}
}

bool UGameplayActionComponent::TryActivateActionBySpec(const FGameplayActionSpec& ActionSpec)
{
	if (AddedActions.Contains(ActionSpec) && IsValid(ActionSpec.Action))
	{
		return ActionSpec.Action->RequestExecuteAction();
	}
	else
	{
		return false;
	}
}

bool UGameplayActionComponent::TryActivateActionsByTag(FGameplayTagContainer ActionTags)
{
	bool bActivatedAny = false;
	
	for (const FGameplayActionSpec& ActionSpec : AddedActions)
	{
		if (IsValid(ActionSpec.Action) && ActionSpec.Action->GetActionTag().MatchesAny(ActionTags))
		{
			if (ActionSpec.Action->RequestExecuteAction())
			{
				bActivatedAny = true;
			}
		}
	}
	
	return bActivatedAny;
}

void UGameplayActionComponent::TriggerActionByClass(TSubclassOf<UGameplayAction> ActionClass, const FInstancedStruct& Payload, int32 ActionLevel)
{
	if (!ActionClass || !GameplayActionActorInfo.IsValid())
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("TriggerActionByClass: Invalid ActionClass or GameplayActionActorInfo."));
		return;
	}

	UGameplayAction* NewAction = NewObject<UGameplayAction>(this, ActionClass);
	if (!IsValid(NewAction))
	{
		UE_LOG(LogGameplayActionComponent, Error, TEXT("TriggerActionByClass: Failed to create new action object of class %s."), *ActionClass->GetName());
		return;
	}

	if (!NewAction->RequestTriggerAction(GameplayActionActorInfo, ActionLevel, Payload))
	{
	    NewAction->MarkAsGarbage();
	}
}

void UGameplayActionComponent::PressInputID(int32 InputID)
{
	const FGameplayActionSpec* ActionSpec = AddedActions.FindByPredicate([InputID](const FGameplayActionSpec& Spec)
	{
		return Spec.InputID == InputID;
	});

	if (ActionSpec && IsValid(ActionSpec->Action) && !ActionSpec->Action->IsActive())
	{
		ActionSpec->Action->RequestExecuteAction();
	}
}

void UGameplayActionComponent::ReleaseInputID(int32 InputID)
{
	const FGameplayActionSpec* ActionSpec = AddedActions.FindByPredicate([InputID](const FGameplayActionSpec& Spec)
	{
		return Spec.InputID == InputID;
	});

	if (ActionSpec && IsValid(ActionSpec->Action))
	{
		ActionSpec->Action->RequestEndAction();
	}
}

void UGameplayActionComponent::AddActiveAction(UGameplayAction* Action)
{
	const FGameplayActionSpec* ExistingActionSpec = AddedActions.FindByPredicate([Action](const FGameplayActionSpec& Spec)
	{
		return Spec.Action == Action;
	});
	
	if (Action && ExistingActionSpec && !ActiveActions.Contains(Action))
	{
		ActiveActions.Add(Action);
		OnActionActivated.Broadcast(Action->GetClass());
	}
}

void UGameplayActionComponent::RemoveActiveAction(UGameplayAction* Action, bool bWasCanceled)
{
	if (Action)
	{
		ActiveActions.Remove(Action);
		OnActionEnded.Broadcast(Action->GetClass(), bWasCanceled);
	}
}

void UGameplayActionComponent::AddTriggeredAction(UGameplayAction* Action)
{
	if (IsValid(Action))
	{
		ActiveTriggeredActions.Add(Action);
		OnActionActivated.Broadcast(Action->GetClass());
	}
}

void UGameplayActionComponent::RemoveTriggeredAction(UGameplayAction* Action, bool bWasCanceled)
{
	if (IsValid(Action))
	{
		ActiveTriggeredActions.Remove(Action);
		OnActionEnded.Broadcast(Action->GetClass(), bWasCanceled);
	}
}

// --- Tag Management ---
void UGameplayActionComponent::AddOwnedGameplayTags(FGameplayTagContainer Tags)
{
	if (GameplayTagContainer.IsValid())
	{
		GameplayTagContainer->AppendTags(Tags);

		OnGameplayTagsAdded.Broadcast(Tags);
	}
}

void UGameplayActionComponent::RemoveOwnedGameplayTags(FGameplayTagContainer Tags)
{
	if (GameplayTagContainer.IsValid())
	{
		GameplayTagContainer->RemoveTags(Tags);

		OnGameplayTagsRemoved.Broadcast(Tags);
	}
}

// --- Attribute Management ---
UGameplayAttributeSet* UGameplayActionComponent::GetAttributeSetByClass(TSubclassOf<UGameplayAttributeSet> AttributeClass) const
{
	if (!AttributeClass) 
	{
	    UE_LOG(LogGameplayAction, Error, TEXT("GetAttributeSetByClass: AttributeClass is nullptr"));
	    return nullptr;
	}

	for (UGameplayAttributeSet* Set : AttributeSets)
	{
		if (IsValid(Set) && Set->IsA(AttributeClass))
		{
			return Set;
		}
	}
	return nullptr;
}

float UGameplayActionComponent::GetAttributeValue(const FGameplayAttribute& Attribute) const
{
	if (!Attribute.IsValid())
	{
		return 0.0f;
	}

	const UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(Attribute.GetAttributeSetClass());

	if (AttributeSet == nullptr)
	{
		UE_LOG(LogGameplayAction, Error, TEXT("GetAttributeValue: AttributeSet is nullptr"));
		return 0.0f;
	}
	
	return AttributeSet->GetNumericValue(Attribute);
}

float UGameplayActionComponent::GetAttributeBaseValue(const FGameplayAttribute& Attribute) const
{
    if (!Attribute.IsValid())
	{
		UE_LOG(LogGameplayAction, Error, TEXT("GetAttributeBaseValue: Attribute is invalid"));
		return 0.0f;
	}
   
	const UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(Attribute.GetAttributeSetClass());
   
	if (AttributeSet == nullptr)
	{
		return 0.0f;
	}

	return AttributeSet->GetBaseValue(Attribute);
}

void UGameplayActionComponent::SetAttributeValue(const FGameplayAttribute& Attribute, float NewValue)
{
	if (!Attribute.IsValid())
	{
		UE_LOG(LogGameplayAction, Error, TEXT("SetAttributeValue: Attribute is invalid"));
		return;
	}

	UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(Attribute.GetAttributeSetClass());

	if (AttributeSet)
	{
		AttributeSet->SetNumericValue(Attribute, NewValue);
	}
}

UGameplayAttributeSet* UGameplayActionComponent::AddAttributeSet(TSubclassOf<UGameplayAttributeSet> AttributeSetClass)
{
	if (!AttributeSetClass)
	{
		return nullptr;
	}
	
	if (UGameplayAttributeSet* ExistingSet = GetAttributeSetByClass(AttributeSetClass))
	{
		return ExistingSet;
	}

	UGameplayAttributeSet* NewAttributeSet = NewObject<UGameplayAttributeSet>(GetOwner(), AttributeSetClass);
	AttributeSets.Add(NewAttributeSet);
	return NewAttributeSet;
}

void UGameplayActionComponent::InitActorInfo()
{
	FGameplayActionActorInfo ActorInfo;

	if (!GetOwner())
	{
		UE_LOG(LogGameplayAction, Error, TEXT("InitActorInfo: GetOwner() is nullptr"));
		return;
	}
	
	ActorInfo.OwnerActor = GetOwner();
	
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	GetOwner()->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
	ActorInfo.SkeletalMeshComponents.Append(SkeletalMeshComponents);
	
	ActorInfo.MovementComponent = GetOwner()->GetComponentByClass<UMovementComponent>();
	ActorInfo.ActionComponent = this;
	
	GameplayActionActorInfo = MakeShared<FGameplayActionActorInfo>(ActorInfo);
	GameplayTagContainer = MakeShared<FGameplayTagContainer>();
}

void UGameplayActionComponent::InitAttributes()
{
	for (const TSubclassOf<UGameplayAttributeSet>& AttributeClass : DefaultAttributes)
	{
		if (AttributeClass)
		{
			AddAttributeSet(AttributeClass); 
		}
	}

	for (const FAttributeInitializationData& InitData : InitialAttributeValues)
	{
		if (!InitData.Attribute.IsValid())
		{
			continue;
		}
		
		UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(InitData.Attribute.GetAttributeSetClass());

		if (AttributeSet)
		{
			AttributeSet->InitNumericValue(InitData.Attribute, InitData.InitialValue);
		}
	}
}

void UGameplayActionComponent::InitActions()
{
    for (const FActionInitializationData& InitData : InitialActions)
    {
        if (!IsValid(InitData.Action))
        {
            continue;
        }

        AddGameplayAction(InitData.Action, InitData.Level);
    }
}

// --- Gameplay Effect Management ---

FActiveGameplayEffectHandle UGameplayActionComponent::ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& Spec)
{
    if (!Spec.Effect) 
    {
        UE_LOG(LogGameplayAction, Error, TEXT("ApplyGameplayEffectSpecToSelf: Spec.Effect is nullptr"));
        return FActiveGameplayEffectHandle();
    }

    const bool HaveAllTags = GameplayTagContainer->HasAll(Spec.Effect->ApplicationRequiredTags);
   
    const bool HaveBlockedTags = GameplayTagContainer->HasAny(Spec.Effect->ApplicationBlockedTags);
    
    if (!HaveAllTags || HaveBlockedTags)
    {
        return FActiveGameplayEffectHandle();
    }
    
    FActiveGameplayEffectHandle ExistingHandle = FindStackableEffect(Spec);

    if (ExistingHandle.IsValid()) 
    {
        return ExistingHandle;
    }

    FActiveGameplayEffect ActiveGE;
    ActiveGE.Handle.Handle = NextEffectHandle++;
    ActiveGE.Spec = Spec;
    ActiveGE.StartTime = GetWorld()->GetTimeSeconds();
    ActiveGE.Duration = Spec.Effect->DurationPolicy == EGameplayEffectDuration::HasDuration ? Spec.Effect->DurationMagnitude : -1.f;
    ActiveGE.StackCount = 1;

    if (Spec.Effect->DurationPolicy != EGameplayEffectDuration::Instant && Spec.Effect->Period <= 0.0f) 
    {
        for (const FGameplayModifierInfo& ModInfo : Spec.Effect->Modifiers)
        {
            if (ModInfo.Attribute.IsValid())
            {
                const float BaseValue = GetAttributeBaseValue(ModInfo.Attribute);
                
                ActiveGE.PreApplyBaseValues.Add(ModInfo.Attribute, BaseValue);
            }
        }
    }

    for (const FGameplayModifierInfo& ModInfo : Spec.Effect->Modifiers)
    {
        if (!ModInfo.Attribute.IsValid()) 
        {
            continue;
        }

        float FinalMagnitude = ModInfo.Magnitude;
        
        if (IsValid(ModInfo.MagnitudeCalculation))
        {
            UGameplayEffectMagnitudeCalculation* CalcCDO = ModInfo.MagnitudeCalculation->GetDefaultObject<UGameplayEffectMagnitudeCalculation>();
            FinalMagnitude = CalcCDO->CalculateMagnitude(Spec, this);
        }
        else if (ModInfo.DataTag.IsValid() && Spec.SetByCallerMagnitudes.Contains(ModInfo.DataTag))
        {
            FinalMagnitude = Spec.GetSetByCallerMagnitude(ModInfo.DataTag);
        }

        const float CurrentValue = GetAttributeValue(ModInfo.Attribute);
        float NewValue = CurrentValue;

        switch (ModInfo.ModifierOp) 
        {
            case EGameplayModOp::Additive:
                NewValue += FinalMagnitude;
                break;

            case EGameplayModOp::Multiplicative:
                NewValue *= FinalMagnitude;
                break;

            case EGameplayModOp::Override:
                NewValue = FinalMagnitude;
                break;
        }

        UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(ModInfo.Attribute.GetAttributeSetClass());

        if (!AttributeSet)
        {
            continue;
        }

        if (Spec.Effect->DurationPolicy == EGameplayEffectDuration::Instant || Spec.Effect->Period > 0.0f) 
        {
            AttributeSet->SetNumericValue(ModInfo.Attribute, NewValue);
        }
        else 
        {
            AttributeSet->SetBaseValue(ModInfo.Attribute, NewValue);
        }
    }

    if (Spec.Effect->ExecutionCalculation)
    {
        UGameplayEffectExecutionCalculation* ExecCDO = Spec.Effect->ExecutionCalculation->GetDefaultObject<UGameplayEffectExecutionCalculation>();
        ExecCDO->Execute(Spec, this);
    }

    AddOwnedGameplayTags(Spec.Effect->GrantedTags);

    if (Spec.Effect->DurationPolicy == EGameplayEffectDuration::Instant) 
    {
        return ActiveGE.Handle;
    }

    ActiveEffects.Add(ActiveGE);

    if (Spec.Effect->DurationPolicy == EGameplayEffectDuration::HasDuration && Spec.Effect->DurationMagnitude > 0.f)
    {
        FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UGameplayActionComponent::OnEffectExpired, ActiveGE.Handle);
        
        GetWorld()->GetTimerManager().SetTimer(ActiveGE.DurationHandle, Delegate, Spec.Effect->DurationMagnitude, false);
    }

    if (Spec.Effect->Period > 0.f) 
    {
        FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UGameplayActionComponent::ExecutePeriodicEffect, ActiveGE.Handle);

        if (Spec.Effect->bExecutePeriodicOnApplication) 
        {
            GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
        }
        GetWorld()->GetTimerManager().SetTimer(ActiveGE.PeriodTimerHandle, Delegate, Spec.Effect->Period, true);
    }

    return ActiveGE.Handle;
}

FActiveGameplayEffectHandle UGameplayActionComponent::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect)
{
    if (!IsValid(Effect))
    {
        return FActiveGameplayEffectHandle();
    }
 
    const UGameplayEffect* EffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
 
    if (!IsValid(EffectCDO))
    {
        UE_LOG(LogGameplayAction, Error, TEXT("ApplyGameplayEffectToSelf: EffectCDO is nullptr"));
        return FActiveGameplayEffectHandle();
    }
 
    FGameplayEffectSpec Spec;
    Spec.Effect = EffectCDO;
 
    return ApplyGameplayEffectSpecToSelf(Spec);
}

bool UGameplayActionComponent::RemoveActiveGameplayEffect(const FActiveGameplayEffectHandle& Handle)
{
    for (int32 Index = 0; Index < ActiveEffects.Num(); ++Index)
    {
        FActiveGameplayEffect& Effect = ActiveEffects[Index];

        if (Effect.Handle == Handle) 
        {
            if (Effect.DurationHandle.IsValid())
            {
                GetWorld()->GetTimerManager().ClearTimer(Effect.DurationHandle);
            }

            if (Effect.PeriodTimerHandle.IsValid()) 
            {
                GetWorld()->GetTimerManager().ClearTimer(Effect.PeriodTimerHandle);
            }

            RevertEffectModifiers(Effect);
            
            RemoveOwnedGameplayTags(Effect.Spec.Effect->GrantedTags);

            ActiveEffects.RemoveAtSwap(Index);

            return true;
        }
    }

    return false;
}

bool UGameplayActionComponent::HasActiveEffectSpecHandle(const FActiveGameplayEffectHandle& Spec)
{
    if (!Spec.IsValid()) 
    {
        return false;
    }

    const FActiveGameplayEffect* ActiveEffect = ActiveEffects.FindByPredicate([Spec](const FActiveGameplayEffect& Effect)
    {
        return Effect.Handle == Spec;
    });

    return ActiveEffect != nullptr;
}

bool UGameplayActionComponent::HasActiveEffect(TSubclassOf<UGameplayEffect> EffectClass)
{
    if (!IsValid(EffectClass)) 
    {
        return false;
    }

    const UGameplayEffect* CDO = EffectClass->GetDefaultObject<UGameplayEffect>();

    if (!IsValid(CDO)) 
    {
        return false;
    }

    const FActiveGameplayEffect* ActiveEffect = ActiveEffects.FindByPredicate([CDO](const FActiveGameplayEffect& Effect)
    {
        return Effect.Spec.Effect == CDO;
    });

    return ActiveEffect != nullptr;
}

void UGameplayActionComponent::RevertEffectModifiers(const FActiveGameplayEffect& Effect)
{
    for (const TPair<FGameplayAttribute, float>& Pair : Effect.PreApplyBaseValues)
    {
        UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(Pair.Key.GetAttributeSetClass());

        if (AttributeSet)
        {
            AttributeSet->SetBaseValue(Pair.Key, Pair.Value);
        }
    }
}

void UGameplayActionComponent::OnEffectExpired(FActiveGameplayEffectHandle Handle)
{
    RemoveActiveGameplayEffect(Handle);
}

void UGameplayActionComponent::ExecutePeriodicEffect(FActiveGameplayEffectHandle Handle)
{
    for (const FActiveGameplayEffect& Effect : ActiveEffects)
    {
        if (Effect.Handle == Handle)
        {
            for (const FGameplayModifierInfo& ModInfo : Effect.Spec.Effect->Modifiers)
            {
                if (!ModInfo.Attribute.IsValid())
                {
                    continue;
                }

                float FinalMagnitude = ModInfo.Magnitude;

                if (IsValid(ModInfo.MagnitudeCalculation))
                {
                    UGameplayEffectMagnitudeCalculation* CalcCDO = ModInfo.MagnitudeCalculation->GetDefaultObject<UGameplayEffectMagnitudeCalculation>();
                    FinalMagnitude = CalcCDO->CalculateMagnitude(Effect.Spec, this);
                }
                else if (ModInfo.DataTag.IsValid() && Effect.Spec.SetByCallerMagnitudes.Contains(ModInfo.DataTag))
                {
                    FinalMagnitude = Effect.Spec.GetSetByCallerMagnitude(ModInfo.DataTag);
                }

                const float CurrentValue = GetAttributeValue(ModInfo.Attribute);

                float NewValue = CurrentValue;

                switch (ModInfo.ModifierOp)
                {
                    case EGameplayModOp::Additive:
                        NewValue += FinalMagnitude;
                        break;

                    case EGameplayModOp::Multiplicative:
                        NewValue *= FinalMagnitude;
                        break;

                    case EGameplayModOp::Override:
                        NewValue = FinalMagnitude;
                        break;
                }

                UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(ModInfo.Attribute.GetAttributeSetClass());

                if (AttributeSet)
                {
                    AttributeSet->SetNumericValue(ModInfo.Attribute, NewValue);
                }
            }

            break;
        }
    }
}

FActiveGameplayEffectHandle UGameplayActionComponent::FindStackableEffect(const FGameplayEffectSpec& Spec) const
{
    if (Spec.Effect->StackingPolicy == EGameplayEffectStacking::None)
    {
        return FActiveGameplayEffectHandle();
    }

    for (const FActiveGameplayEffect& Effect : ActiveEffects)
    {
        if (Effect.Spec.Effect == Spec.Effect)
        {
            return Effect.Handle;
        }
    }

    return FActiveGameplayEffectHandle();
}

void UGameplayActionComponent::DisplayDebug(class UCanvas* Canvas, float& YL, float& YPos) const
{
    if (!Canvas)
    {
        return;
    }

    UFont* Font = GEngine->GetSmallFont();
    YL = Font->GetMaxCharHeight() + 2.f;
    const float XPos = 4.f;

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, TEXT("--- Grant Actions ---"), XPos, YPos);
    YPos += YL;

    for (const FGameplayActionSpec& Spec : AddedActions)
    {
        if (!Spec.IsValid())
        {
            continue;
        }

        const FColor Color = Spec.Action->IsActive() ? FColor::Green : FColor::White;
        Canvas->SetDrawColor(Color);
        Canvas->DrawText(Font, FString::Printf(TEXT("  %s"), *Spec.Action->GetClass()->GetName()), XPos + 8.f, YPos);
        YPos += YL;
    }

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, TEXT("--- Triggered Actions ---"), XPos, YPos);
    YPos += YL;

    for (const UGameplayAction* Action : ActiveTriggeredActions)
    {
        if (!IsValid(Action))
        {
            continue;
        }

        Canvas->SetDrawColor(FColor::Green);
        Canvas->DrawText(Font, FString::Printf(TEXT("  %s"), *Action->GetClass()->GetName()), XPos + 8.f, YPos);
        YPos += YL;
    }

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, TEXT("--- Active Effects ---"), XPos, YPos);
    YPos += YL;

    for (const FActiveGameplayEffect& Effect : ActiveEffects)
    {
        if (!Effect.Spec.Effect)
        {
            continue;
        }

        FString Info = FString::Printf(TEXT("  %s [Stacks: %d]"), *Effect.Spec.Effect->GetName(), Effect.StackCount);

        if (Effect.Duration > 0.f)
        {
            const float TimeLeft = Effect.Duration - (GetWorld()->GetTimeSeconds() - Effect.StartTime);
            Info += FString::Printf(TEXT(" (%.1fs)"), FMath::Max(0.f, TimeLeft));
        }
        else
        {
            Info += TEXT(" (Infinite)");
        }

        Canvas->DrawText(Font, Info, XPos + 8.f, YPos);
        YPos += YL;
    }

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, TEXT("--- Owned Tags ---"), XPos, YPos);
    YPos += YL;

    if (GameplayTagContainer.IsValid())
    {
        Canvas->DrawText(Font, FString::Printf(TEXT("  %s"), *GameplayTagContainer->ToStringSimple()), XPos + 8.f, YPos);
        YPos += YL;
    }

    Canvas->SetDrawColor(FColor::White);
    Canvas->DrawText(Font, TEXT("--- Attributes ---"), XPos, YPos);
    YPos += YL;

    for (const UGameplayAttributeSet* Set : AttributeSets)
    {
        if (!IsValid(Set))
        {
            continue;
        }

        Canvas->DrawText(Font, FString::Printf(TEXT("  [%s]"), *Set->GetClass()->GetName()), XPos + 8.f, YPos);
        YPos += YL;

        for (TFieldIterator<FProperty> It(Set->GetClass()); It; ++It)
        {
            const FStructProperty* StructProp = CastField<FStructProperty>(*It);
            if (!StructProp || StructProp->Struct != FGameplayAttributeData::StaticStruct())
            {
                continue;
            }

            const FGameplayAttributeData* Data = StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(const_cast<UGameplayAttributeSet*>(Set));
            if (Data)
            {
                Canvas->DrawText(Font, FString::Printf(TEXT("    %s: Base=%.1f Current=%.1f"), *It->GetName(), Data->GetBaseValue(), Data->GetCurrentValue()), XPos + 16.f, YPos);
                YPos += YL;
            }
        }
    }
}
