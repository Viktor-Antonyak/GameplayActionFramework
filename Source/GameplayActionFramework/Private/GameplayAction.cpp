// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAction.h"
#include "GameFramework/Actor.h"
#include "GameplayActionComponent.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY(LogGameplayAction);

UGameplayAction::UGameplayAction()
{
	ActionType = EGameplayActionType::Default;
	bIsActive = false;
	CachedActionLevel = -1;
}

void UGameplayAction::InitializeAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel)
{
	CachedActorInfo = ActorInfo;
	CachedActionLevel = ActionLevel;
}

void UGameplayAction::DeinitializeAction()
{
	CachedActorInfo.Reset();
	CachedActionLevel = -1;
}

bool UGameplayAction::RequestExecuteAction()
{
    if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        return false;
    }
    
	if (!IsValid(GetActionComponent()))
	{
		UE_LOG(LogGameplayAction, Error, TEXT("RequestExecuteAction: GetActionComponent() is nullptr"));
		return false;
	}
	
	const bool bFoundBlockAction = CheckBlockedActionTags();
	const bool bHasBlockedTags = CheckBlockedTags();
	const bool bHasRequiredTags = CheckRequiredTags();
	const bool bCanceledAllActions = CanCancelAllActions();
	const bool bHasActiveCooldownEffect = GetActionComponent()->HasActiveEffect(CooldownEffect);
	
	if (!bIsActive && CanExecuteAction() && !bFoundBlockAction && 
		(bCanceledAllActions || CancelPolicy == EGameplayActionCancelPolicy::IgnoreFailed) && 
		bHasRequiredTags && !bHasBlockedTags && !bHasActiveCooldownEffect)
	{
		bIsActive = true;
		GetActionComponent()->AddActiveAction(this);
		GetActionComponent()->AddOwnedGameplayTags(GameplayActionInfoTags.GrantTags);
		
		OnExecuteAction();
		return true;
	}
	return false;
}

bool UGameplayAction::RequestTriggerAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel, const FInstancedStruct& Payload)
{
    if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        return false;
    }
    
	if (!ActorInfo.IsValid() || !ActorInfo->ActionComponent.IsValid())
	{
		UE_LOG(LogGameplayAction, Error, TEXT("RequestTriggerAction: ActorInfo is nullptr or ActionComponent is nullptr"));
		return false;
	}

	InitializeAction(ActorInfo, ActionLevel);
	
	FGameplayTag ThisActionTag = GetActionTag();
	
	const bool bFoundBlockAction = CheckBlockedActionTags();
	const bool bHasBlockedTags = CheckBlockedTags();
	const bool bHasRequiredTags = CheckRequiredTags();
	const bool bCanceledAllActions = CanCancelAllActions();
	const bool bHasActiveCooldownEffect = GetActionComponent()->HasActiveEffect(CooldownEffect);
	
	if (!bIsActive && CanExecuteAction() && !bFoundBlockAction && 
		(bCanceledAllActions || CancelPolicy == EGameplayActionCancelPolicy::IgnoreFailed) && 
		bHasRequiredTags && !bHasBlockedTags && !bHasActiveCooldownEffect)
	{
		bIsActive = true;
		GetActionComponent()->AddTriggeredAction(this);
		GetActionComponent()->AddOwnedGameplayTags(GameplayActionInfoTags.GrantTags);
		
		OnActionTriggered(Payload);
		return true;
	}
	return false;
}

void UGameplayAction::RequestEndAction()
{
    if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        return;
    }
    
	if (bIsActive)
	{
		EndAction();
	}
}

bool UGameplayAction::RequestCancelAction()
{
    if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        return false;
    }
    
	if (bIsActive && bCanBeCanceled)
	{
		CancelAction();
		return true;
	}
	return false;
}

UWorld* UGameplayAction::GetWorld() const
{
	if (CachedActorInfo.IsValid() && CachedActorInfo->OwnerActor.IsValid())
	{
		return CachedActorInfo->OwnerActor->GetWorld();
	}
	return nullptr;
}

// Start of IGameplayTaskOwnerInterface
UGameplayTasksComponent* UGameplayAction::GetGameplayTasksComponent(const UGameplayTask& Task) const
{
	return GetActionComponent();
}

AActor* UGameplayAction::GetGameplayTaskOwner(const UGameplayTask* Task) const
{
	return GetOwnerActor();
}

AActor* UGameplayAction::GetGameplayTaskAvatar(const UGameplayTask* Task) const
{
	return GetOwnerActor();
}

void UGameplayAction::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskInitialized(Task);
}

void UGameplayAction::OnGameplayTaskActivated(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskActivated(Task);
}

void UGameplayAction::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskDeactivated(Task);
}
// End of IGameplayTaskOwnerInterface

void UGameplayAction::EndAction()
{
	if (!GetActionComponent() && !bIsActive)
	{
		return;
	}

	bIsActive = false;
	bCanBeCanceled = true;
	
	OnEndAction(false);
	
	if (ActionType == EGameplayActionType::Default)
	{
		GetActionComponent()->RemoveActiveAction(this, false);
	}
	else
	{
		GetActionComponent()->RemoveTriggeredAction(this);
	}
	
	GetActionComponent()->RemoveOwnedGameplayTags(GameplayActionInfoTags.GrantTags);
}

void UGameplayAction::CancelAction()
{
	if (!GetActionComponent() && !bIsActive)
	{
		return;
	}
	
	bIsActive = false;
	bCanBeCanceled = true;
	
	OnEndAction(true);
	
	if (ActionType == EGameplayActionType::Default)
	{
		GetActionComponent()->RemoveActiveAction(this, true);
	}
	else
	{
		GetActionComponent()->RemoveTriggeredAction(this, true);
	}
	
	GetActionComponent()->RemoveOwnedGameplayTags(GameplayActionInfoTags.GrantTags);
}

bool UGameplayAction::CommitCost()
{
	if (!CanApplyCost())
	{
		return false;
	}
	
	return GetActionComponent()->ApplyGameplayEffectToSelf(CostEffect).IsValid();
}

bool UGameplayAction::CommitCooldown()
{
    UGameplayActionComponent* ActionComponent = GetActionComponent();
    
    if (!IsValid(CooldownEffect) || !IsValid(ActionComponent) || ActionComponent->HasActiveEffect(CooldownEffect))
    {
        return false;
    }

    return ActionComponent->ApplyGameplayEffectToSelf(CooldownEffect).IsValid();
}

TArray<USkeletalMeshComponent*> UGameplayAction::GetSkeletalMeshComponents() const
{
	TArray<USkeletalMeshComponent*> RawSkeletalMeshComponents;

	if (CachedActorInfo.IsValid())
	{
		for (const TWeakObjectPtr<USkeletalMeshComponent>& SkeletalMeshComponent : CachedActorInfo->SkeletalMeshComponents)
		{
			if (SkeletalMeshComponent.IsValid())
			{
				RawSkeletalMeshComponents.Add(SkeletalMeshComponent.Get());
			}
		}
	}
	return RawSkeletalMeshComponents;
}

bool UGameplayAction::CanCancelAllActions()
{
    bool bCanceledAllActions = true;
    
    for (UGameplayAction* ActiveAction : GetActionComponent()->GetActiveActions())
    {
        if (ActiveAction && ActiveAction != this && GetCancelOtherActionsTags().HasTag(ActiveAction->GetActionTag()))
        {
            if (!ActiveAction->RequestCancelAction())
            {
                bCanceledAllActions = false;
                
                if (CancelPolicy == EGameplayActionCancelPolicy::Block)
                {
                    break;
                }
            }
        }
    }
    
    return bCanceledAllActions;
}

bool UGameplayAction::CheckBlockedTags()
{
    return GetActionComponent()->GetOwnedGameplayTags().HasAny(GameplayActionInfoTags.BlockedByTags);
}

bool UGameplayAction::CheckRequiredTags()
{
    return GetActionComponent()->GetOwnedGameplayTags().HasAll(GameplayActionInfoTags.RequireTags);
}

bool UGameplayAction::CheckBlockedActionTags()
{
    FGameplayTag ThisActionTag = GetActionTag();
    
   	UGameplayAction* const* FoundBlockAction = GetActionComponent()->GetActiveActions().FindByPredicate(
	[ThisActionTag](const UGameplayAction* PredicateAction)
	{
		return PredicateAction && PredicateAction->GetBlockOtherActionsTags().HasTag(ThisActionTag);
	});

    return FoundBlockAction != nullptr;
}

bool UGameplayAction::CanApplyCost()
{
    UGameplayActionComponent* ActionComponent = GetActionComponent();
    
    if (!IsValid(CostEffect) || !IsValid(ActionComponent))
    {
        return false;
    }

    UGameplayEffect* CostEffectCDO = CostEffect->GetDefaultObject<UGameplayEffect>();

    if (!IsValid(CostEffectCDO))
    {
        return false;
    }

    for (const FGameplayModifierInfo& ModifierInfo : CostEffectCDO->Modifiers)
    {
        if (!ModifierInfo.Attribute.IsValid())
        {
            continue;
        }

        float CostValue = ModifierInfo.Magnitude;

        const float CurrentValue = ActionComponent->GetAttributeValue(ModifierInfo.Attribute);

        float NewValue = CurrentValue;

        switch (ModifierInfo.ModifierOp)
        {
            case EGameplayModOp::Additive:
                NewValue += CostValue;
                break;
                
            case EGameplayModOp::Multiplicative:
                NewValue *= CostValue;
                break;
                
            case EGameplayModOp::Override:
                NewValue = CostValue;
                break;
        }
        
        if (NewValue < 0.0f)
        {
            return false;
        }
    }

    return true;
}

void UGameplayAction::OnExecuteAction_Implementation() {}
void UGameplayAction::OnActionTriggered_Implementation(const FInstancedStruct& Payload) {}
void UGameplayAction::OnEndAction_Implementation(bool bWasCanceled) {}
bool UGameplayAction::CanExecuteAction_Implementation() const { return true; }
