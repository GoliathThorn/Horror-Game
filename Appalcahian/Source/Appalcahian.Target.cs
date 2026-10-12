using UnrealBuildTool;
using System.Collections.Generic;

public class AppalcahianTarget : TargetRules
{
	public AppalcahianTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Appalcahian");
	}
}
