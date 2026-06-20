#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayActionFrameworkDebugSubsystem.generated.h"

UCLASS()
class GAMEPLAYACTIONFRAMEWORK_API UGameplayActionFrameworkDebugSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnDebugDraw(class UCanvas* Canvas, class APlayerController* PlayerController);

private:
	FDelegateHandle DrawHandle;
};
