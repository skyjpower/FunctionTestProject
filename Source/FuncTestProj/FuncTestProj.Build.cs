// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FuncTestProj : ModuleRules
{
	public FuncTestProj(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay", 

			// PropertyVisibility
			"PropertyEditor",
			"SlateCore",

			// PathFollowing
			"AIModule",
			"NavigationSystem",
		});
	}
}
