// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SilentWorld : ModuleRules
{
	public SilentWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "SilentWorld", "SilentWorld/Public" });
		PrivateIncludePaths.AddRange(new string[] { "SilentWorld/Private" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara" });
        
		PublicDependencyModuleNames.AddRange(new string[] { "StructUtils", "MassCommon", "MassEntity", "MassSpawner" });

		if (Target.Type != TargetType.Server)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "SlateCore", "Slate", "UMG" });
		}

		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
    }
}
