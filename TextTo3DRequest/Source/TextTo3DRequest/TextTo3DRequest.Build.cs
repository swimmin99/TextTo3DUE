// Copyright 2025 Devhanghae All Rights Reserved.

using UnrealBuildTool;

public class TextTo3DRequest : ModuleRules
{
    public TextTo3DRequest(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { });
        PrivateIncludePaths.AddRange(new string[] { });

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        // Runtime and Editor
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Projects",
                "InputCore",
                "Json",
                "JsonUtilities",
            }
        );

        // Editor Only
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "UnrealEd",
                    "Slate",
                    "SlateCore",
                    "ToolMenus",
                    "DesktopPlatform"
                }
            );
        }

        DynamicallyLoadedModuleNames.AddRange(new string[] { });
    }
}