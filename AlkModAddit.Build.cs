// Copyright 2020 Alkaline Games, LLC. All Rights Reserved.

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
      "Http",
      "Json"
      // !!! not yet required:
      //"JsonUtilities"
    });
  }
}
