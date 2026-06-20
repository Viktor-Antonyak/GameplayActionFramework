#include "GameplayActionFrameworkDebugSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "ShowFlags.h"
#include "Debug/DebugDrawService.h"
#include "HAL/IConsoleManager.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameplayActionComponent.h"

static TCustomShowFlag<> GGameplayActionFrameworkDebugFlag(TEXT("GAF"), false, SFG_Developer);

static int32 GAFDebugEnabled = 0;

static void OnGAFDebugToggle()
{
	GAFDebugEnabled = !GAFDebugEnabled;

	static IConsoleVariable* FlagVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowFlag.GAF"));
	if (FlagVar)
	{
		FlagVar->Set(GAFDebugEnabled ? 1 : 0, ECVF_SetByConsole);
	}
}

static FAutoConsoleCommand GAFDebugToggleCmd(
	TEXT("DebugGAF"),
	TEXT("Toggle GameplayActionFramework debug visualization"),
	FConsoleCommandDelegate::CreateStatic(&OnGAFDebugToggle)
);

void UGameplayActionFrameworkDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DrawHandle = UDebugDrawService::Register(
		TEXT("GAF"),
		FDebugDrawDelegate::CreateUObject(this, &UGameplayActionFrameworkDebugSubsystem::OnDebugDraw)
	);
}

void UGameplayActionFrameworkDebugSubsystem::Deinitialize()
{
	if (DrawHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawHandle);
		DrawHandle.Reset();
	}

	Super::Deinitialize();
}

void UGameplayActionFrameworkDebugSubsystem::OnDebugDraw(UCanvas* Canvas, APlayerController* PlayerController)
{
	if (!GAFDebugEnabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!PlayerController)
	{
		PlayerController = World->GetFirstPlayerController();
	}

	float YL = 0.f, YPos = 4.f;

	if (PlayerController)
	{
		UFont* Font = GEngine->GetSmallFont();
		YL = Font->GetMaxCharHeight() + 2.f;

		FVector CameraLoc;
		FRotator CameraRot;
		PlayerController->GetPlayerViewPoint(CameraLoc, CameraRot);

		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;

		if (World->LineTraceSingleByChannel(Hit, CameraLoc, CameraLoc + CameraRot.Vector() * 10000.f, ECC_Visibility, QueryParams))
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				if (UGameplayActionComponent* GAC = HitActor->FindComponentByClass<UGameplayActionComponent>())
				{
					Canvas->SetDrawColor(FColor::White);
					Canvas->DrawText(Font, FString::Printf(TEXT("--- %s ---"), *HitActor->GetName()), 4.f, YPos);
					YPos += YL;

					GAC->DisplayDebug(Canvas, YL, YPos);
					return;
				}
			}
		}

		if (APawn* Pawn = PlayerController->GetPawn())
		{
			if (UGameplayActionComponent* GAC = Pawn->FindComponentByClass<UGameplayActionComponent>())
			{
				Canvas->SetDrawColor(FColor::White);
				Canvas->DrawText(Font, FString::Printf(TEXT("--- %s ---"), *Pawn->GetName()), 4.f, YPos);
				YPos += YL;

				GAC->DisplayDebug(Canvas, YL, YPos);
			}
		}
	}
}
