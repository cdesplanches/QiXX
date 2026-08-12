#include "Isometric/IsometricFunctionLibrary.h"

#include "GenericPlatform/GenericPlatformChunkInstall.h"
#include "HAL/PlatformMisc.h"

bool UIsometricFunctionLibrary::IsChunkAvailable(int32 ChunkId)
{
	if (IPlatformChunkInstall* ChunkInstall = FPlatformMisc::GetPlatformChunkInstall())
	{
		const EChunkLocation::Type Location = ChunkInstall->GetPakchunkLocation(ChunkId);
		// Only wait while the chunk is pending installation. A chunk that is not
		// present in this build (editor/PIE, non-chunked build) needs no waiting.
		return Location != EChunkLocation::NotAvailable;
	}
	return true;
}
