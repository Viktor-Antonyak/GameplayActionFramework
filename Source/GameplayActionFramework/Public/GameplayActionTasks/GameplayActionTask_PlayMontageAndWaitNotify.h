// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayActionTask.h"
#include "Animation/AnimInstance.h"
#include "GameplayActionTask_PlayMontageAndWaitNotify.generated.h"

class UAnimMontage;
class USkeletalMeshComponent;

/** @note All delegates must send NotifyName otherwise we won't see him as output pin in task node */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMontageTaskDelegate, FName, NotifyName);

/**
 * Run Animation Montage on a specific Skeletal Mesh Component
 */
UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionTask_PlayMontageAndWaitNotify : public UGameplayActionTask
{
	GENERATED_BODY()
	
public:
	
	/** 
	 * Plays an Animation Montage on the given Skeletal Mesh Component and waits for branching point notifies.
	 * @param InMesh The mesh whose AnimInstance plays the montage.
	 * @param Montage The montage to play.
	 * @param PlayRate Playback rate (1.0 is normal speed).
	 * @param StartSection Optional section name to jump to at start (ignored if none).
	 * @param StartTimeSeconds Time offset in seconds to start the montage at.
	 * @param bStopMontageAtEnd If true, stops the montage instead of leaving it playing when the task is destroyed early.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayAction | Tasks", 
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGameplayActionTask_PlayMontageAndWaitNotify* PlayMontageAndWaitNotify
	(
		UObject* WorldContextObject, 
		USkeletalMeshComponent* InMesh,
		UAnimMontage* Montage, 
		float PlayRate = 1.0f, 
		FName StartSection = NAME_None,
		float StartTimeSeconds = 0.0f,
		bool bStopMontageAtEnd = false
	);
	
	/** Fired when the montage finishes playing naturally. */
	UPROPERTY(BlueprintAssignable)
	FOnMontageTaskDelegate OnCompleted;
	 
	/** Fired when the montage starts blending out towards another animation. */
	UPROPERTY(BlueprintAssignable)
	FOnMontageTaskDelegate OnBlendedOut;
	 
	/** Fired when the montage is interrupted (e.g. replaced by another montage). */
	UPROPERTY(BlueprintAssignable)
	FOnMontageTaskDelegate OnInterrupted;
	 
	/** Fired when the task is destroyed before the montage ended naturally. */
	UPROPERTY(BlueprintAssignable)
	FOnMontageTaskDelegate OnCancelled;
	
	/** 
	* Fired when a Branching Point notify begins during the montage. 
	* @note only fires for notifies explicitly marked as "Branching Point" in the montage editor 
	* regular AnimNotify/AnimNotifyState events on standard tracks will NOT trigger this delegate. 
	*/
	UPROPERTY(BlueprintAssignable)
	FOnMontageTaskDelegate OnNotifyBegin;
	
	/** 
	* Fired when a Branching Point notify ends during the montage. 
	* @note only fires for notifies explicitly marked as "Branching Point" in the montage editor 
	* regular AnimNotify/AnimNotifyState events on standard tracks will NOT trigger this delegate. 
	*/
	UPROPERTY(BlueprintAssignable)
	FOnMontageTaskDelegate OnNotifyEnd;
	
protected:
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	 
	void OnMontageBlendedOut(UAnimMontage* Montage, bool bInterrupted);
	
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
		
	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
		
	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> TargetMesh;
	 
	UPROPERTY()
	TObjectPtr<UAnimMontage> MontageToPlay;
	 
	float MontagePlayRate;
	
	FName MontageStartSection;
	
	float MontageStartTimeSeconds;
	
	bool bStopAtEnd;
	
	bool bMontageEndedNaturally = false;
		
	TWeakObjectPtr<UAnimInstance> CachedAnimInstance;
};
