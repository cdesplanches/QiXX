#include "BlueprintMCPEditorSubsystem.h"
#include "BlueprintMCPServer.h"

void UBlueprintMCPEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Don't start in commandlet mode — the commandlet has its own server instance.
	if (IsRunningCommandlet())
	{
		return;
	}

	Server = MakeUnique<FBlueprintMCPServer>();
	if (Server->Start(9847, /*bEditorMode=*/true))
	{
		UE_LOG(LogTemp, Display, TEXT("BlueprintMCP: Editor subsystem started — MCP server on port %d"), Server->GetPort());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BlueprintMCP: Editor subsystem failed to start MCP server (port may be in use)"));
		Server.Reset();
	}
}

void UBlueprintMCPEditorSubsystem::Deinitialize()
{
	if (Server)
	{
		Server->Stop();
		Server.Reset();
		UE_LOG(LogTemp, Display, TEXT("BlueprintMCP: Editor subsystem stopped."));
	}

	Super::Deinitialize();
}

void UBlueprintMCPEditorSubsystem::Tick(float DeltaTime)
{
	if (Server)
	{
		Server->ProcessOneRequest();
	}
}

bool UBlueprintMCPEditorSubsystem::IsTickable() const
{
	return Server.IsValid() && Server->IsRunning();
}

TStatId UBlueprintMCPEditorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UBlueprintMCPEditorSubsystem, STATGROUP_Tickables);
}
