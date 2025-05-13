// Copyright © 2020 - 2025 Alkaline Games, LLC. All rights reserved.
//
using UnrealBuildTool;

public class AlkModAddit : ModuleRules
{
  public AlkModAddit(ReadOnlyTargetRules Target) : base(Target)
  {
    bLegacyPublicIncludePaths = false;
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    PrivateDependencyModuleNames.AddRange(new string[] {
      "Core",
      "CoreUObject",
      "Engine",
      "HTTP",
      "Json"
      // !!! not yet required:
      //"JsonUtilities"
    });
  }
}
