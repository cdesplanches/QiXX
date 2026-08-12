#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IsometricFunctionLibrary.generated.h"

/**
 * Blueprint function library with shared isometric utilities
 */
UCLASS()
class QIXX_API UIsometricFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns true when the given chunk id is usable right now: either it is
	 * installed and mountable, or it is not part of this build (no download to wait for).
	 * Only returns false while the chunk is pending installation.
	 */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Chunks")
	static bool IsChunkAvailable(int32 ChunkId);
};
