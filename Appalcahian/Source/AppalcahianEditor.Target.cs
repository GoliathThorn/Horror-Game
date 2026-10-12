using UnrealBuildTool;
using System.Collections.Generic;

public class AppalcahianEditorTarget : TargetRules
{
	public AppalcahianEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Appalcahian");
	}
}
