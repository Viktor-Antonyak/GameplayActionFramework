// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/MovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayActionTypes.h"
#include "GameplayActionAttributeSet.h"
#include "GameplayAction.h" 
#include "GameplayActionEffect.h"
#include "GameplayActionEffectMagnitudeCalc.h"
#include "GameplayActionEffectExecutionCalc.h"


DEFINE_LOG_CATEGORY(LogGameplayActionComponent);

UGameplayActionComponent::UGameplayActionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
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
	GameplayTagContainer.Empty();

	const TArray<UGameplayAction*> ActiveActionsCopy = ActiveActions;
	for (UGameplayAction* ActiveAction : ActiveActionsCopy)
	{
		if (IsValid(ActiveAction))
		{
			ActiveAction->RequestEndAction();
		}
	}

	const TArray<UGameplayAction*> ActiveTriggeredActionsCopy = ActiveTriggeredActions;
	for (UGameplayAction* TriggeredAction : ActiveTriggeredActionsCopy)
	{
		if (IsValid(TriggeredAction))
		{
			TriggeredAction->RequestEndAction();
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
	if (DefaultAction && DefaultAction->GetActionType() == EGameplayActionType::Triggered)
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("AddGameplayAction: Action class %s is a Triggered type and cannot be added as a permanent action."), *Action->GetName());
		return FGameplayActionSpec();
	}
	
	UGameplayAction* NewAction = NewObject<UGameplayAction>(this, Action);
	
	FGameplayActionSpec NewActionSpec;
	NewActionSpec.Action = NewAction;
	NewActionSpec.Level = Level;
	NewActionSpec.InputID = NewAction->GetActionInputID();
	
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
	
	const TArray<FGameplayActionSpec> AddedActionsCopy = AddedActions;
	for (const FGameplayActionSpec& ActionSpec : AddedActionsCopy)
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
	
	const UGameplayAction* DefaultAction = ActionClass->GetDefaultObject<UGameplayAction>();
	if (DefaultAction && DefaultAction->GetActionType() == EGameplayActionType::Default)
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("TriggerActionByClass: Action class %s is a Default type and cannot be added as a triggered action."), *ActionClass->GetName());
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
		ActionSpec->Action->RequestReleaseInputID();
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

bool UGameplayActionComponent::CanActionCancelOtherActions(const UGameplayAction* Action) const
{
	bool bCanceledAllActions = true;
    
	TArray<UGameplayAction*> Actions = GetActiveActions();
	
	for (UGameplayAction* ActiveAction : Actions)
	{
		if (ActiveAction && ActiveAction != Action && Action->GetCancelOtherActionsTags().HasTag(ActiveAction->GetActionTag()))
		{
			if (!ActiveAction->RequestCancelAction())
			{
				bCanceledAllActions = false;
                
				if (Action->GetCancelPolicy() == EGameplayActionCancelPolicy::Block)
				{
					break;
				}
			}
		}
	}
	
	return bCanceledAllActions;
}

bool UGameplayActionComponent::ActionBlockedByOtherAction(const UGameplayAction* Action) const
{
	FGameplayTag ThisActionTag = Action->GetActionTag();
    
	TArray<UGameplayAction*> Actions = ActiveActions;
	
	UGameplayAction* const* FoundBlockAction = Actions.FindByPredicate(
		[ThisActionTag](const UGameplayAction* PredicateAction)
		{
			return PredicateAction && PredicateAction->GetBlockOtherActionsTags().HasTag(ThisActionTag);
		});
	
	return FoundBlockAction;
}

// --- Tag Management ---
void UGameplayActionComponent::AddOwnedGameplayTags(FGameplayTagContainer Tags, int32 Quantity)
{
	if (!Tags.IsValid() || Quantity <=0)
	{
		return;
	}

	GameplayTagContainer.AddTags(Tags, Quantity);
	
	OnGameplayTagsAdded.Broadcast(Tags);
}

void UGameplayActionComponent::RemoveOwnedGameplayTags(FGameplayTagContainer Tags, int32 Quantity)
{
	if (!Tags.IsValid() || Quantity <= 0)
	{
		return;
	}
	
	GameplayTagContainer.RemoveTags(Tags, Quantity);
	
	OnGameplayTagsRemoved.Broadcast(Tags);
}

void UGameplayActionComponent::SendGameplayEvent(FGameplayTag EventTag)
{
	if (EventTag.IsValid())
	{
		OnGameplayActionEvent.Broadcast(EventTag);	
	}
}

// --- Attribute Management ---
UGameplayActionAttributeSet* UGameplayActionComponent::GetAttributeSetByClass(TSubclassOf<UGameplayActionAttributeSet> AttributeClass) const
{
	if (!AttributeClass) 
	{
	    UE_LOG(LogGameplayActionComponent, Error, TEXT("GetAttributeSetByClass: AttributeClass is nullptr"));
	    return nullptr;
	}

	for (UGameplayActionAttributeSet* Set : AttributeSets)
	{
		if (IsValid(Set) && Set->IsA(AttributeClass))
		{
			return Set;
		}
	}
	return nullptr;
}

float UGameplayActionComponent::GetAttributeValue(const FGameplayActionAttribute& Attribute) const
{
	if (!Attribute.IsValid())
	{
		return 0.0f;
	}

	const UGameplayActionAttributeSet* AttributeSet = GetAttributeSetByClass(Attribute.GetAttributeSetClass());

	if (AttributeSet == nullptr)
	{
		UE_LOG(LogGameplayActionComponent, Error, TEXT("GetAttributeValue: AttributeSet is nullptr"));
		return 0.0f;
	}
	
	return AttributeSet->GetNumericValue(Attribute);
}

float UGameplayActionComponent::GetAttributeBaseValue(const FGameplayActionAttribute& Attribute) const
{
    if (!Attribute.IsValid())
	{
		UE_LOG(LogGameplayActionComponent, Error, TEXT("GetAttributeBaseValue: Attribute is invalid"));
		return 0.0f;
	}
   
	const UGameplayActionAttributeSet* AttributeSet = GetAttributeSetByClass(Attribute.GetAttributeSetClass());
   
	if (AttributeSet == nullptr)
	{
		return 0.0f;
	}

	return AttributeSet->GetBaseValue(Attribute);
}

void UGameplayActionComponent::SetAttributeValue(const FGameplayActionAttribute& Attribute, float NewValue)
{
	if (!Attribute.IsValid())
	{
		UE_LOG(LogGameplayActionComponent, Error, TEXT("SetAttributeValue: Attribute is invalid"));
		return;
	}

	UGameplayActionAttributeSet* AttributeSet = GetAttributeSetByClass(Attribute.GetAttributeSetClass());

	if (AttributeSet)
	{
		AttributeSet->SetNumericValue(Attribute, NewValue);
	}
}

UGameplayActionAttributeSet* UGameplayActionComponent::AddAttributeSet(TSubclassOf<UGameplayActionAttributeSet> AttributeSetClass)
{
	if (!AttributeSetClass)
	{
		return nullptr;
	}
	
	if (UGameplayActionAttributeSet* ExistingSet = GetAttributeSetByClass(AttributeSetClass))
	{
		return ExistingSet;
	}

	UGameplayActionAttributeSet* NewAttributeSet = NewObject<UGameplayActionAttributeSet>(GetOwner(), AttributeSetClass);
    NewAttributeSet->SetOwningActionComponent(this);
	AttributeSets.Add(NewAttributeSet);
	return NewAttributeSet;
}

void UGameplayActionComponent::InitActorInfo()
{
	FGameplayActionActorInfo ActorInfo;

	if (!GetOwner())
	{
		UE_LOG(LogGameplayActionComponent, Error, TEXT("InitActorInfo: GetOwner() is nullptr"));
		return;
	}
	
	ActorInfo.OwnerActor = GetOwner();
	
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	GetOwner()->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
	ActorInfo.SkeletalMeshComponents.Append(SkeletalMeshComponents);
	
	ActorInfo.MovementComponent = GetOwner()->GetComponentByClass<UMovementComponent>();
	ActorInfo.ActionComponent = this;
	
	GameplayActionActorInfo = MakeShared<FGameplayActionActorInfo>(ActorInfo);
}

void UGameplayActionComponent::InitAttributes()
{
	for (const TSubclassOf<UGameplayActionAttributeSet>& AttributeClass : DefaultAttributes)
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
		
		UGameplayActionAttributeSet* AttributeSet = GetAttributeSetByClass(InitData.Attribute.GetAttributeSetClass());

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

FActiveGameplayActionEffectHandle UGameplayActionComponent::ApplyGameplayEffectSpecToSelf(const FGameplayActionEffectSpec& Spec)
{
    if (!Spec.Effect) 
    {
        UE_LOG(LogGameplayActionComponent, Error, TEXT("ApplyGameplayEffectSpecToSelf: Spec.Effect is nullptr"));
        return FActiveGameplayActionEffectHandle();
    }

    const bool HaveAllTags = GameplayTagContainer.HasAllMatchingTags(Spec.Effect->ApplicationRequiredTags);
   
    const bool HaveBlockedTags = GameplayTagContainer.HasAnyMatchingTags(Spec.Effect->ApplicationBlockedTags);
    
    if (!HaveAllTags || HaveBlockedTags)
    {
        return FActiveGameplayActionEffectHandle();
    }
    
    FActiveGameplayActionEffectHandle ExistingHandle = ActiveEffects.FindStackable(Spec.Effect, Spec.SourceActor.Get());

    if (ExistingHandle.IsValid()) 
    {
    	FActiveGameplayActionEffect* ExistingEffect = ActiveEffects.Find(ExistingHandle);
    	if (ExistingEffect)
    	{
    		if (ExistingEffect->StackCount >= Spec.Effect->StackLimitCount)
    		{
    			return ExistingHandle;
    		}

    		ExistingEffect->StackCount++;
    		ExistingEffect->Spec = Spec;

    		ApplyEffectModifiers(*ExistingEffect);

    		return ExistingHandle;
    	}
    }

    FActiveGameplayActionEffect ActiveGE;
    ActiveGE.Handle.Handle = NextEffectHandle++;
    ActiveGE.Spec = Spec;
    ActiveGE.StartTime = GetWorld()->GetTimeSeconds();
    ActiveGE.Duration = Spec.Effect->DurationPolicy == EGameplayActionEffectDuration::HasDuration ? Spec.Effect->DurationMagnitude : -1.f;
    ActiveGE.StackCount = 1;

    ApplyEffectModifiers(ActiveGE);

    if (Spec.Effect->ExecutionCalculation)
    {
        UGameplayActionEffectExecutionCalc* ExecCDO = Spec.Effect->ExecutionCalculation->GetDefaultObject<UGameplayActionEffectExecutionCalc>();
        ExecCDO->Execute(Spec, this);
    }

    AddOwnedGameplayTags(Spec.Effect->GrantedTags);

    if (Spec.Effect->DurationPolicy == EGameplayActionEffectDuration::Instant) 
    {
        return ActiveGE.Handle;
    }

    ActiveEffects.Add(ActiveGE);
	
	OnEffectAdded.Broadcast(Spec.Effect->GetClass());

	if (Spec.Effect->DurationPolicy == EGameplayActionEffectDuration::HasDuration && Spec.Effect->DurationMagnitude > 0.f)
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

FActiveGameplayActionEffectHandle UGameplayActionComponent::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayActionEffect> Effect, int32 Level, AActor* SourceActor)
{
    if (!IsValid(Effect))
    {
        return FActiveGameplayActionEffectHandle();
    }
 
    const UGameplayActionEffect* EffectCDO = Effect->GetDefaultObject<UGameplayActionEffect>();
 
    if (!IsValid(EffectCDO))
    {
        UE_LOG(LogGameplayActionComponent, Error, TEXT("ApplyGameplayEffectToSelf: EffectCDO is nullptr"));
        return FActiveGameplayActionEffectHandle();
    }
 
    FGameplayActionEffectSpec Spec;
    Spec.Effect = EffectCDO;
	Spec.Level = Level;
	Spec.SourceActor = SourceActor;
 
    return ApplyGameplayEffectSpecToSelf(Spec);
}

bool UGameplayActionComponent::RemoveActiveGameplayEffect(const FActiveGameplayActionEffectHandle& Handle)
{
	FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	
	if (!Effect)
	{
		return false;
	}

	if (Effect->DurationHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(Effect->DurationHandle);
	}
	if (Effect->PeriodTimerHandle.IsValid()) 
	{
		GetWorld()->GetTimerManager().ClearTimer(Effect->PeriodTimerHandle);
	}

	RevertEffectModifiers(*Effect);
	
	RemoveOwnedGameplayTags(Effect->Spec.Effect->GrantedTags);

	TSubclassOf<UGameplayActionEffect> EffectClass = Effect->Spec.Effect->GetClass();

	ActiveEffects.Remove(Handle);

	OnEffectRemoved.Broadcast(EffectClass);

	return true;
}

bool UGameplayActionComponent::HasActiveEffect(TSubclassOf<UGameplayActionEffect> EffectClass) const
{
	if (!IsValid(EffectClass)) 
	{
		return false;
	}
	
	const UGameplayActionEffect* CDO = EffectClass->GetDefaultObject<UGameplayActionEffect>();
	
	if (!IsValid(CDO)) 
	{
		return false;
	}

	for (const TTuple<FActiveGameplayActionEffectHandle, FActiveGameplayActionEffect>& Pair : ActiveEffects.GetAll())
	{
		if (Pair.Value.Spec.Effect == CDO)
		{
			return true;
		}
	}
	
	return false;
}

bool UGameplayActionComponent::SetEffectInhibited(const FActiveGameplayActionEffectHandle& Handle, bool bInhibited)
{
	FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	
	if (!Effect || Effect->bIsInhibited == bInhibited)
	{
		return false;
	}

	if (Effect->Spec.Effect->DurationPolicy == EGameplayActionEffectDuration::Instant)
	{
		UE_LOG(LogGameplayActionComponent, Warning, TEXT("SetEffectInhibited: Cannot inhibit an Instant effect, it has no ongoing modifiers to toggle."));
		
		return false;
	}

	Effect->bIsInhibited = bInhibited;

	if (bInhibited)
	{
		RevertEffectModifiers(*Effect);
	}
	else
	{
		ApplyEffectModifiers(*Effect);
	}

	return true;
}

bool UGameplayActionComponent::IsEffectInhibited(const FActiveGameplayActionEffectHandle& Handle) const
{
	const FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	return Effect ? Effect->bIsInhibited : false;
}

float UGameplayActionComponent::GetEffectElapsedTime(const FActiveGameplayActionEffectHandle& Handle) const
{
	const FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	
	if (!Effect || !GetWorld())
	{
		return 0.0f;
	}
	
	return GetWorld()->GetTimeSeconds() - Effect->StartTime;
}

float UGameplayActionComponent::GetEffectRemainingTime(const FActiveGameplayActionEffectHandle& Handle) const
{
	const FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	
	if (!Effect || Effect->Duration <= 0.0f)
	{
		return -1.0f;
	}
	
	return FMath::Max(0.0f, Effect->Duration - GetEffectElapsedTime(Handle));
}

float UGameplayActionComponent::GetEffectDuration(const FActiveGameplayActionEffectHandle& Handle) const
{
	const FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	
	return Effect ? Effect->Duration : 0.0f;
}

void UGameplayActionComponent::ApplyEffectModifiers(const FActiveGameplayActionEffect& Effect)
{
	const FGameplayActionEffectSpec& Spec = Effect.Spec;

	for (const FGameplayActionEffectModifierInfo& ModInfo : Spec.Effect->Modifiers)
	{
		if (!ModInfo.Attribute.IsValid())
		{
			continue;
		}

		float FinalMagnitude = ModInfo.Magnitude;

		if (IsValid(ModInfo.MagnitudeCalculation))
		{
			
			UGameplayActionEffectMagnitudeCalc* CalcCDO = ModInfo.MagnitudeCalculation->GetDefaultObject<UGameplayActionEffectMagnitudeCalc>();
			FinalMagnitude = CalcCDO->CalculateMagnitude(Spec, this);
		}
		else if (ModInfo.DataTag.IsValid() && Spec.SetByCallerMagnitudes.Contains(ModInfo.DataTag))
		{
			FinalMagnitude = Spec.GetSetByCallerMagnitude(ModInfo.DataTag);
		}

		UGameplayActionAttributeSet* AttributeSet = GetAttributeSetByClass(ModInfo.Attribute.GetAttributeSetClass());
		if (!AttributeSet)
		{
			continue;
		}

		if (Spec.Effect->DurationPolicy == EGameplayActionEffectDuration::Instant || Spec.Effect->Period > 0.0f)
		{
			const float CurrentValue = AttributeSet->GetBaseValue(ModInfo.Attribute);
			
			float NewValue = CurrentValue;

			switch (ModInfo.ModifierOp)
			{
			case EGameplayActionEffectModOp::Additive:      
				NewValue += FinalMagnitude;
				break;
			
			case EGameplayActionEffectModOp::Multiplicative:
				NewValue *= FinalMagnitude;
				break;
				
			case EGameplayActionEffectModOp::Override:
				NewValue = FinalMagnitude;
				break;
			}

			AttributeSet->SetBaseValue(ModInfo.Attribute, NewValue);
		}
		else
		{
			AttributeSet->ApplyModifier(ModInfo.Attribute, Effect.Handle, ModInfo.ModifierOp, FinalMagnitude);
		}
	}
}

void UGameplayActionComponent::RevertEffectModifiers(const FActiveGameplayActionEffect& Effect)
{
	for (const FGameplayActionEffectModifierInfo& ModInfo : Effect.Spec.Effect->Modifiers)
	{
		if (!ModInfo.Attribute.IsValid())
		{
			continue;
		}

		UGameplayActionAttributeSet* AttributeSet = GetAttributeSetByClass(ModInfo.Attribute.GetAttributeSetClass());
		
		if (AttributeSet)
		{
			AttributeSet->RemoveModifiersFromSource(ModInfo.Attribute, Effect.Handle);
		}
	}
}

void UGameplayActionComponent::OnEffectExpired(FActiveGameplayActionEffectHandle Handle)
{
    RemoveActiveGameplayEffect(Handle);
}

void UGameplayActionComponent::ExecutePeriodicEffect(FActiveGameplayActionEffectHandle Handle)
{
	FActiveGameplayActionEffect* Effect = ActiveEffects.Find(Handle);
	
	if (!Effect || Effect->bIsInhibited)  
	{
		return;
	
	}
	ApplyEffectModifiers(*Effect);
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
    const float StartYPos = YPos;

    TArray<TPair<FString, FColor>> Lines;

    Lines.Add({TEXT("--- Grant Actions ---"), FColor::White});
    for (const FGameplayActionSpec& Spec : AddedActions)
    {
        if (Spec.IsValid())
        {
            const FColor Color = Spec.Action->IsActive() ? FColor::Green : FColor::White;
            Lines.Add({FString::Printf(TEXT("  %s"), *Spec.Action->GetClass()->GetName()), Color});
        }
    }

    Lines.Add({TEXT("--- Triggered Actions ---"), FColor::White});
    for (const UGameplayAction* Action : ActiveTriggeredActions)
    {
        if (IsValid(Action))
        {
            Lines.Add({FString::Printf(TEXT("  %s"), *Action->GetClass()->GetName()), FColor::Green});
        }
    }

    Lines.Add({TEXT("--- Active Effects ---"), FColor::White});
    for (const TTuple<FActiveGameplayActionEffectHandle, FActiveGameplayActionEffect>& Pair : ActiveEffects.GetAll())
    {
        const FActiveGameplayActionEffect& Effect = Pair.Value;
        if (!Effect.Spec.Effect) { continue; }

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

        if (Effect.bIsInhibited)
        {
            Info += TEXT(" [INHIBITED]");
        }

        const FColor Color = Effect.bIsInhibited ? FColor::Silver : FColor::White;
        Lines.Add({Info, Color});
    }

    Lines.Add({TEXT("--- Owned Tags ---"), FColor::White});
    if (GameplayTagContainer.IsValid())
    {
        Lines.Add({FString::Printf(TEXT("  %s"), *GameplayTagContainer.ToStringAggregated()), FColor::White});
    }

    Lines.Add({TEXT("--- Attributes ---"), FColor::White});
    for (const UGameplayActionAttributeSet* Set : AttributeSets)
    {
        if (!IsValid(Set)) { continue; }

        Lines.Add({FString::Printf(TEXT("  [%s]"), *Set->GetClass()->GetName()), FColor::White});

        for (TFieldIterator<FProperty> It(Set->GetClass()); It; ++It)
        {
            const FStructProperty* StructProp = CastField<FStructProperty>(*It);
            if (!StructProp || StructProp->Struct != FGameplayActionAttributeData::StaticStruct()) { continue; }

            const FGameplayActionAttributeData* Data = StructProp->ContainerPtrToValuePtr<FGameplayActionAttributeData>(const_cast<UGameplayActionAttributeSet*>(Set));
            if (Data)
            {
                Lines.Add({FString::Printf(TEXT("    %s: Base=%.1f Current=%.1f"), *It->GetName(), Data->GetBaseValue(), Data->GetCurrentValue()), FColor::White});
            }
        }
    }

    float MaxWidth = 0.f;
    for (const TPair<FString, FColor>& LinePair : Lines)
    {
        float LineWidth, LineHeight;
        Canvas->TextSize(Font, LinePair.Key, LineWidth, LineHeight);
        MaxWidth = FMath::Max(MaxWidth, LineWidth);
    }

    const float BackgroundWidth = MaxWidth + 16.f;
    const float BackgroundHeight = Lines.Num() * YL + 8.f;

    FCanvasTileItem BackgroundTile(
        FVector2D(XPos - 4.f, StartYPos - 2.f),
        FVector2D(BackgroundWidth, BackgroundHeight),
        FLinearColor(0.f, 0.f, 0.f, 0.2f)
    );
    BackgroundTile.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(BackgroundTile);

    for (const TPair<FString, FColor>& LinePair : Lines)
    {
        Canvas->SetDrawColor(LinePair.Value);
        Canvas->DrawText(Font, LinePair.Key, XPos + (LinePair.Key.StartsWith(TEXT("---")) ? 0.f : 8.f), YPos);
        YPos += YL;
    }
}
