// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.


#include "GameplayActionTasks/GameplayActionTask_PlayMontageAndWaitNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"

UGameplayActionTask_PlayMontageAndWaitNotify::UGameplayActionTask_PlayMontageAndWaitNotify()
{
	bTickingTask = false;
}

UGameplayActionTask_PlayMontageAndWaitNotify* UGameplayActionTask_PlayMontageAndWaitNotify::PlayMontageAndWaitNotify
(
	UObject* WorldContextObject, 
	USkeletalMeshComponent* InMesh, 
	UAnimMontage* Montage, 
	float PlayRate,
	FName StartSection, 
	float StartTimeSeconds,
	bool bStopMontageAtEnd
)
{
	UGameplayActionTask_PlayMontageAndWaitNotify* NewTask = NewGameplayActionTask<UGameplayActionTask_PlayMontageAndWaitNotify>(WorldContextObject);

	if (NewTask)
	{
		NewTask->TargetMesh = InMesh;
		NewTask->MontageToPlay = Montage;
		NewTask->MontagePlayRate = PlayRate;
		NewTask->MontageStartSection = StartSection;
		NewTask->MontageStartTimeSeconds = StartTimeSeconds;
		NewTask->bStopAtEnd = bStopMontageAtEnd;
	}
	
	return NewTask;
}

void UGameplayActionTask_PlayMontageAndWaitNotify::Activate()
{
	if (!TargetMesh || !MontageToPlay)
	{
		EndTask();
		return;
	}
	 
	UAnimInstance* AnimInstance = TargetMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		EndTask();
		return;
	}
	
	CachedAnimInstance = AnimInstance;
	
	Super::Activate();
	
	float Duration = AnimInstance->Montage_Play(MontageToPlay, MontagePlayRate, EMontagePlayReturnType::Duration, MontageStartTimeSeconds);
	if (Duration <= 0.0f)
	{
		EndTask();
		return;
	}
	 
	if (MontageStartSection != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(MontageStartSection, MontageToPlay);
	}
	
	FOnMontageBlendingOutStarted BlendedOutDelegate;
	BlendedOutDelegate.BindUObject(this, &UGameplayActionTask_PlayMontageAndWaitNotify::OnMontageBlendedOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendedOutDelegate, MontageToPlay);
	 
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UGameplayActionTask_PlayMontageAndWaitNotify::OnMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
	
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UGameplayActionTask_PlayMontageAndWaitNotify::OnNotifyBeginReceived);
	AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UGameplayActionTask_PlayMontageAndWaitNotify::OnNotifyEndReceived);
}

void UGameplayActionTask_PlayMontageAndWaitNotify::OnDestroy(bool bInOwnerFinished)
{
	if (CachedAnimInstance.IsValid())
	{
		CachedAnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGameplayActionTask_PlayMontageAndWaitNotify::OnNotifyBeginReceived);
		CachedAnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &UGameplayActionTask_PlayMontageAndWaitNotify::OnNotifyEndReceived);
	 
		if (!bMontageEndedNaturally)
		{
			OnCancelled.Broadcast(NAME_None);
			OnCanceledNative.Broadcast(NAME_None);
			
			if (bStopAtEnd && MontageToPlay)
			{
				CachedAnimInstance->Montage_Stop(MontageToPlay->GetDefaultBlendOutTime(), MontageToPlay);
			}
		}
	}
	 
	Super::OnDestroy(bInOwnerFinished);
}

void UGameplayActionTask_PlayMontageAndWaitNotify::OnMontageBlendedOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == MontageToPlay)
	{
		if (bInterrupted)
		{
			OnInterrupted.Broadcast(NAME_None);
			OnInterruptedNative.Broadcast(NAME_None);
		}
		else
		{
			OnBlendedOut.Broadcast(NAME_None);
			OnBlendedOutNative.Broadcast(NAME_None);
		}
	}
}

void UGameplayActionTask_PlayMontageAndWaitNotify::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == MontageToPlay)
	{
	
		bMontageEndedNaturally = true;
	
		if (!bInterrupted)
		{
			OnCompleted.Broadcast(NAME_None);
			OnCompletedNative.Broadcast(NAME_None);
		}
		
		EndTask();
	}
}

void UGameplayActionTask_PlayMontageAndWaitNotify::OnNotifyBeginReceived(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (BranchingPointPayload.SequenceAsset == MontageToPlay)
	{
		OnNotifyBegin.Broadcast(NotifyName);
		OnNotifyBeginNative.Broadcast(NotifyName);
	}
}

void UGameplayActionTask_PlayMontageAndWaitNotify::OnNotifyEndReceived(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (BranchingPointPayload.SequenceAsset == MontageToPlay)
	{
		OnNotifyEnd.Broadcast(NotifyName);
		OnNotifyEndNative.Broadcast(NotifyName);
	}
}
