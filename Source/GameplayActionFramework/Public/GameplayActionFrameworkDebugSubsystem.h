// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayActionFrameworkDebugSubsystem.generated.h"

UCLASS()
class UE_DEPRECATED(1.6, "UGameplayActionFrameworkDebugSubsystem is deprecated since GAF 1.6, use the Gameplay Debugger 'GAF' category instead.")
GAMEPLAYACTIONFRAMEWORK_API UGameplayActionFrameworkDebugSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Draws debug information for all actions, active effects, and attributes on the canvas. */
	void OnDebugDraw(class UCanvas* Canvas, class APlayerController* PlayerController);

private:
	FDelegateHandle DrawHandle;
};
