// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayAction.h"
#include "GameFramework/Actor.h"
#include "GameplayActionComponent.h"
#include "GameplayActionFragment.h"

DEFINE_LOG_CATEGORY(LogGameplayAction);

UGameplayAction::UGameplayAction()
{
}

void UGameplayAction::InitializeAction(const TSharedPtr<FGameplayActionActorInfo>& ActorInfo, int32 ActionLevel)
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}

	if (!ActorInfo.IsValid())
	{
		return;
	}
	
	if (bIsActive)
	{
		return;
	}
	
	CachedActorInfo = ActorInfo;
	CachedActionLevel = ActionLevel;
	
	OnActionInitialize();
}

void UGameplayAction::DeinitializeAction()
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}
	
	if (bIsActive)
	{
		return;
	}
	
	CachedActorInfo.Reset();
	CachedActionLevel = -1;
	
	OnActionDeinitialize();
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
	const bool bHasActiveCooldownEffect = GetActionComponent()->HasActiveEffect(CooldownEffect);
	
	if (bIsActive || !CanExecuteAction() || bFoundBlockAction || !bHasRequiredTags || bHasBlockedTags || bHasActiveCooldownEffect)
	{
		return false;
	}
    
	const bool bCanceledAllActions = CanCancelAllActions();
	if (!bCanceledAllActions && CancelPolicy == EGameplayActionCancelPolicy::Block)
	{
		return false;
	}
    
	bIsActive = true;
	GetActionComponent()->AddActiveAction(this);
	GetActionComponent()->AddOwnedGameplayTags(GameplayActionInfoTags.GrantTags);
    
	OnExecuteAction();
	return true;
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
	
	const bool bFoundBlockAction = CheckBlockedActionTags();
	const bool bHasBlockedTags = CheckBlockedTags();
	const bool bHasRequiredTags = CheckRequiredTags();
	const bool bHasActiveCooldownEffect = GetActionComponent()->HasActiveEffect(CooldownEffect);
	
	if (bIsActive || !CanExecuteAction() || bFoundBlockAction || !bHasRequiredTags || bHasBlockedTags || bHasActiveCooldownEffect)
	{
		return false;
	}
	
	const bool bCanceledAllActions = CanCancelAllActions();
	if (!bCanceledAllActions && CancelPolicy == EGameplayActionCancelPolicy::Block)
	{
		return false;
	}
	
	bIsActive = true;
	GetActionComponent()->AddTriggeredAction(this);
	GetActionComponent()->AddOwnedGameplayTags(GameplayActionInfoTags.GrantTags);
		
	OnActionTriggered(Payload);
	return true;
}

void UGameplayAction::RequestReleaseInputID()
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}

	if (bIsActive)
	{
		OnInputIDReleased();
	}
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
	if (CachedActorInfo.IsValid() && CachedActorInfo->OwnerActor.IsValid() && !HasAnyFlags(RF_ClassDefaultObject))
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
	GameplayTasks.AddUnique(&Task);
}

void UGameplayAction::OnGameplayTaskActivated(UGameplayTask& Task)
{
}

void UGameplayAction::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	GameplayTasks.Remove(&Task);
}
// End of IGameplayTaskOwnerInterface

void UGameplayAction::EndAction()
{
	if (!GetActionComponent() || !bIsActive)
	{
		return;
	}

	bIsActive = false;
	
	bCanBeCanceled = true;
	
	EndAllGameplayTasks();
	
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
	if (!GetActionComponent() || !bIsActive)
	{
		return;
	}
	
	bIsActive = false;
	
	bCanBeCanceled = true;
	
	EndAllGameplayTasks();
	
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

bool UGameplayAction::CommitCost(int32 CostLevel)
{
	if (!CanApplyCost())
	{
		return false;
	}
	
	return GetActionComponent()->ApplyGameplayEffectToSelf(CostEffect, CostLevel).IsValid();
}

bool UGameplayAction::CommitCooldown(int32 CooldownLevel)
{
    UGameplayActionComponent* ActionComponent = GetActionComponent();
    
    if (!IsValid(CooldownEffect) || !IsValid(ActionComponent) || ActionComponent->HasActiveEffect(CooldownEffect))
    {
        return false;
    }

    return ActionComponent->ApplyGameplayEffectToSelf(CooldownEffect, CooldownLevel).IsValid();
}

TArray<USkeletalMeshComponent*> UGameplayAction::GetSkeletalMeshComponents() const
{
	TArray<USkeletalMeshComponent*> Components;
	
	if (CachedActorInfo.IsValid())
	{
		for (const TWeakObjectPtr<USkeletalMeshComponent>& SkeletalMeshComponent : CachedActorInfo->SkeletalMeshComponents)
		{
			if (SkeletalMeshComponent.IsValid())
			{
				Components.Add(SkeletalMeshComponent.Get());
			}
		}
	}
	
	return Components;
}

bool UGameplayAction::CanCancelAllActions() const 
{
	if (!IsValid(GetActionComponent()))
	{
		return false;
	}
    
    return GetActionComponent()->CanActionCancelOtherActions(this);
}

bool UGameplayAction::CheckBlockedTags() const 
{
	if (!IsValid(GetActionComponent()))
	{
		return false;
	}
	
    return GetActionComponent()->GetOwnedGameplayTags().HasAny(GameplayActionInfoTags.BlockedByTags);
}

bool UGameplayAction::CheckRequiredTags() const 
{
	if (!IsValid(GetActionComponent()))
	{
		return false;
	}
	
    return GetActionComponent()->GetOwnedGameplayTags().HasAll(GameplayActionInfoTags.RequireTags);
}

bool UGameplayAction::CheckBlockedActionTags() const 
{
    if (!IsValid(GetActionComponent()))
    {
	    return false;
    }

    return GetActionComponent()->ActionBlockedByOtherAction(this);
}

bool UGameplayAction::CanApplyCost() const
{
    UGameplayActionComponent* ActionComponent = GetActionComponent();
    
    if (!IsValid(CostEffect) || !IsValid(ActionComponent))
    {
        return false;
    }

    UGameplayActionEffect* CostEffectCDO = CostEffect->GetDefaultObject<UGameplayActionEffect>();

    if (!IsValid(CostEffectCDO))
    {
        return false;
    }

    for (const FGameplayActionEffectModifierInfo& ModifierInfo : CostEffectCDO->Modifiers)
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
            case EGameplayActionEffectModOp::Additive:
                NewValue += CostValue;
                break;
                
            case EGameplayActionEffectModOp::Multiplicative:
                NewValue *= CostValue;
                break;
                
            case EGameplayActionEffectModOp::Override:
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

void UGameplayAction::EndAllGameplayTasks()
{
	const TArray<UGameplayTask*> TasksCopy = GameplayTasks;
	for (UGameplayTask* Task : TasksCopy)
	{
		if (IsValid(Task))
		{
			Task->EndTask();
		}
	}
}
