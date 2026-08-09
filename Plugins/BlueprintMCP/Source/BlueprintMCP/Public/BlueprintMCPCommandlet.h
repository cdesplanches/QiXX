#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "BlueprintMCPServer.h"
#include "BlueprintMCPCommandlet.generated.h"

/**
 * Standalone commandlet that hosts the Blueprint MCP HTTP server.
 * Delegates all logic to FBlueprintMCPServer and runs a manual engine tick loop.
 *
 * Usage:  UnrealEditor-Cmd.exe Project.uproject -run=BlueprintMCP [-port=9847]
 */
UCLASS()
class UBlueprintMCPCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UBlueprintMCPCommandlet();
	virtual int32 Main(const FString& Params) override;

private:
	TUniquePtr<FBlueprintMCPServer> Server;
};
