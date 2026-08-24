// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameplayActionFramework.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayActionFrameworkDebugger.h"
#endif

#define LOCTEXT_NAMESPACE "FGameplayActionFrameworkModule"

void FGameplayActionFrameworkModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& DebuggerModule = IGameplayDebugger::Get();
	DebuggerModule.RegisterCategory("GAF", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGAFGameplayDebuggerCategory::MakeInstance));
	DebuggerModule.NotifyCategoriesChanged();
#endif
}

void FGameplayActionFrameworkModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (FModuleManager::Get().IsModuleLoaded("GameplayDebugger"))
	{
		IGameplayDebugger& DebuggerModule = IGameplayDebugger::Get();
		DebuggerModule.UnregisterCategory("GAF");
		DebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameplayActionFrameworkModule, GameplayActionFramework)
