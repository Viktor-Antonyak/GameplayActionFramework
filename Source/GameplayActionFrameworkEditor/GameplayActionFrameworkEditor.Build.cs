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
                "GameplayActionFramework",
                "EngineAssetDefinitions", 
                "AssetDefinition",       
                "AssetTools",            
                "UnrealEd"
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
                "ContentBrowser"
            }
        );
    }
}
