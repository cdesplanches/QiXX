#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MenuGameState.generated.h"

class APlayerController;

/**
 * Game state for menu and lobby maps.
 * Replicates the set of ready players so every client sees the lobby status.
 */
UCLASS()
class QIXX_API AMenuGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMenuGameState();

	/** Names of the players who pressed "ready", replicated to all clients. */
	UPROPERTY(ReplicatedUsing = OnRep_ReadyPlayerNames, BlueprintReadOnly, Category = "QiXX|Menu")
	TArray<FString> ReadyPlayerNames;

	UFUNCTION()
	void OnRep_ReadyPlayerNames();

	/** Rebuilds ReadyPlayerNames from the given ready controller set (server only). */
	void SetReadyPlayers(const TArray<APlayerController*>& ReadyPlayers);

	/** True when every connected player is ready and at least two players are present. */
	bool AreAllPlayersReady() const;

	/** Fills player names + ready states for the lobby UI. */
	void GetLobbyPlayerList(TArray<FString>& OutNames, TArray<bool>& OutReady) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
