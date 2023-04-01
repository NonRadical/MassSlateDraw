// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SilentWorldTarget : TargetRules
{
	public SilentWorldTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("SilentWorld");
		GlobalDefinitions.Add("FORCE_USE_STATS=1"); //GIVE ME STATS!!
	}
}
