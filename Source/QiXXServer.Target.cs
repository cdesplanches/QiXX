using UnrealBuildTool;
using System.Collections.Generic;

public class QiXXServerTarget : TargetRules
{
    public QiXXServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        ExtraModuleNames.Add("QiXX");
    }
}
