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
#include "UObject/Object.h"


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
	if (DefaultAction && DefaultAction->ActionType == EGameplayActionType::Default)
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

int32 UGameplayActionComponent::GetOwnedGameplayTagCount(FGameplayTag Tag) const
{
	return GameplayTagContainer.GetTagCount(Tag);
}

// --- Attribute Management ---
UGameplayAttributeSet* UGameplayActionComponent::GetAttributeSetByClass(TSubclassOf<UGameplayAttributeSet> AttributeClass) const
{
	if (!AttributeClass) 
	{
	    UE_LOG(LogGameplayActionComponent, Error, TEXT("GetAttributeSetByClass: AttributeClass is nullptr"));
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
		UE_LOG(LogGameplayActionComponent, Error, TEXT("GetAttributeValue: AttributeSet is nullptr"));
		return 0.0f;
	}
	
	return AttributeSet->GetNumericValue(Attribute);
}

float UGameplayActionComponent::GetAttributeBaseValue(const FGameplayAttribute& Attribute) const
{
    if (!Attribute.IsValid())
	{
		UE_LOG(LogGameplayActionComponent, Error, TEXT("GetAttributeBaseValue: Attribute is invalid"));
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
		UE_LOG(LogGameplayActionComponent, Error, TEXT("SetAttributeValue: Attribute is invalid"));
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
        UE_LOG(LogGameplayActionComponent, Error, TEXT("ApplyGameplayEffectSpecToSelf: Spec.Effect is nullptr"));
        return FActiveGameplayEffectHandle();
    }

    const bool HaveAllTags = GameplayTagContainer.HasAllMatchingTags(Spec.Effect->ApplicationRequiredTags);
   
    const bool HaveBlockedTags = GameplayTagContainer.HasAnyMatchingTags(Spec.Effect->ApplicationBlockedTags);
    
    if (!HaveAllTags || HaveBlockedTags)
    {
        return FActiveGameplayEffectHandle();
    }
    
    FActiveGameplayEffectHandle ExistingHandle = ActiveEffects.FindStackable(Spec.Effect, Spec.SourceActor.Get());

    if (ExistingHandle.IsValid()) 
    {
    	FActiveGameplayEffect* ExistingEffect = ActiveEffects.Find(ExistingHandle);
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

    FActiveGameplayEffect ActiveGE;
    ActiveGE.Handle.Handle = NextEffectHandle++;
    ActiveGE.Spec = Spec;
    ActiveGE.StartTime = GetWorld()->GetTimeSeconds();
    ActiveGE.Duration = Spec.Effect->DurationPolicy == EGameplayEffectDuration::HasDuration ? Spec.Effect->DurationMagnitude : -1.f;
    ActiveGE.StackCount = 1;

    ApplyEffectModifiers(ActiveGE);

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
	
	OnEffectAdded.Broadcast(Spec.Effect->GetClass());

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

FActiveGameplayEffectHandle UGameplayActionComponent::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, int32 Level, AActor* SourceActor)
{
    if (!IsValid(Effect))
    {
        return FActiveGameplayEffectHandle();
    }
 
    const UGameplayEffect* EffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
 
    if (!IsValid(EffectCDO))
    {
        UE_LOG(LogGameplayActionComponent, Error, TEXT("ApplyGameplayEffectToSelf: EffectCDO is nullptr"));
        return FActiveGameplayEffectHandle();
    }
 
    FGameplayEffectSpec Spec;
    Spec.Effect = EffectCDO;
	Spec.Level = Level;
	Spec.SourceActor = SourceActor;
 
    return ApplyGameplayEffectSpecToSelf(Spec);
}

bool UGameplayActionComponent::RemoveActiveGameplayEffect(const FActiveGameplayEffectHandle& Handle)
{
	FActiveGameplayEffect* Effect = ActiveEffects.Find(Handle);
	
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

	TSubclassOf<UGameplayEffect> EffectClass = Effect->Spec.Effect->GetClass();

	ActiveEffects.Remove(Handle);

	OnEffectRemoved.Broadcast(EffectClass);

	return true;
}

bool UGameplayActionComponent::HasActiveEffectSpecHandle(const FActiveGameplayEffectHandle& Spec)
{
	return Spec.IsValid() && ActiveEffects.Find(Spec) != nullptr;
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

	for (const TTuple<FActiveGameplayEffectHandle, FActiveGameplayEffect>& Pair : ActiveEffects.GetAll())
	{
		if (Pair.Value.Spec.Effect == CDO)
		{
			return true;
		}
	}
	
	return false;
}

bool UGameplayActionComponent::SetEffectInhibited(const FActiveGameplayEffectHandle& Handle, bool bInhibited)
{
	FActiveGameplayEffect* Effect = ActiveEffects.Find(Handle);
	
	if (!Effect || Effect->bIsInhibited == bInhibited)
	{
		return false;
	}

	if (Effect->Spec.Effect->DurationPolicy == EGameplayEffectDuration::Instant)
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

bool UGameplayActionComponent::IsEffectInhibited(const FActiveGameplayEffectHandle& Handle) const
{
	const FActiveGameplayEffect* Effect = ActiveEffects.Find(Handle);
	return Effect ? Effect->bIsInhibited : false;
}

void UGameplayActionComponent::ApplyEffectModifiers(const FActiveGameplayEffect& Effect)
{
	const FGameplayEffectSpec& Spec = Effect.Spec;

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

		UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(ModInfo.Attribute.GetAttributeSetClass());
		if (!AttributeSet)
		{
			continue;
		}

		if (Spec.Effect->DurationPolicy == EGameplayEffectDuration::Instant || Spec.Effect->Period > 0.0f)
		{
			const float CurrentValue = AttributeSet->GetBaseValue(ModInfo.Attribute);
			
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

			AttributeSet->SetBaseValue(ModInfo.Attribute, NewValue);
		}
		else
		{
			AttributeSet->ApplyModifier(ModInfo.Attribute, Effect.Handle, ModInfo.ModifierOp, FinalMagnitude);
		}
	}
}

void UGameplayActionComponent::RevertEffectModifiers(const FActiveGameplayEffect& Effect)
{
	for (const FGameplayModifierInfo& ModInfo : Effect.Spec.Effect->Modifiers)
	{
		if (!ModInfo.Attribute.IsValid())
		{
			continue;
		}

		UGameplayAttributeSet* AttributeSet = GetAttributeSetByClass(ModInfo.Attribute.GetAttributeSetClass());
		
		if (AttributeSet)
		{
			AttributeSet->RemoveModifiersFromSource(ModInfo.Attribute, Effect.Handle);
		}
	}
}

void UGameplayActionComponent::OnEffectExpired(FActiveGameplayEffectHandle Handle)
{
    RemoveActiveGameplayEffect(Handle);
}

void UGameplayActionComponent::ExecutePeriodicEffect(FActiveGameplayEffectHandle Handle)
{
	FActiveGameplayEffect* Effect = ActiveEffects.Find(Handle);
	
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
    for (const TTuple<FActiveGameplayEffectHandle, FActiveGameplayEffect>& Pair : ActiveEffects.GetAll())
    {
        const FActiveGameplayEffect& Effect = Pair.Value;
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
    for (const UGameplayAttributeSet* Set : AttributeSets)
    {
        if (!IsValid(Set)) { continue; }

        Lines.Add({FString::Printf(TEXT("  [%s]"), *Set->GetClass()->GetName()), FColor::White});

        for (TFieldIterator<FProperty> It(Set->GetClass()); It; ++It)
        {
            const FStructProperty* StructProp = CastField<FStructProperty>(*It);
            if (!StructProp || StructProp->Struct != FGameplayAttributeData::StaticStruct()) { continue; }

            const FGameplayAttributeData* Data = StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(const_cast<UGameplayAttributeSet*>(Set));
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
