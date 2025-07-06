// Copyright 2025 James Vanas
// MIT License – see LICENSE file for details.

using UnrealBuildTool;
using System.IO;

public class PCGCavalierContours : ModuleRules
{
	public PCGCavalierContours(ReadOnlyTargetRules Target) : base(Target)
	{
		bWarningsAsErrors = false;

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{


			}
		);

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../CavalierContours/Include"));


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"PCG",				
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{			
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"Settings"
				});
		}
	}
}