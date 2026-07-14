// Copyright (c) 2026 Viktor Antonyak. All Rights Reserved.
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

using UnrealBuildTool;

public class GameplayActionFrameworkEditor : ModuleRules
{
    public GameplayActionFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "GameplayActionFramework"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "PropertyEditor",
                "EditorStyle",
                "InputCore",
                "GraphEditor",
                "BlueprintGraph",
                "Kismet",            
                "KismetWidgets",
                "KismetCompiler",
                "ToolMenus",
                "ContentBrowser",
                "EngineAssetDefinitions", 
                "AssetDefinition",       
                "AssetTools",            
                "UnrealEd"
            }
        );
    }
}
