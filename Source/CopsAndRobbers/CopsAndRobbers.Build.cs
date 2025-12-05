// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CopsAndRobbers : ModuleRules
{
	public CopsAndRobbers(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			// Intial Dependency
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",

			// UI	
			"UMG", "Slate", "SlateCore",

			// AI
			"AIModule", "NavigationSystem",
        });

        PublicIncludePaths.AddRange(new string[]
        {
            "CopsAndRobbers", // 헤더 Include 시, 경로 탐색용
		});
    }
}
