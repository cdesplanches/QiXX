#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuGameMode.generated.h"

class APlayerController;

/**
 * Game mode for menu and lobby maps. No pawns, no gameplay.
 * Tracks lobby "ready" state and starts the game (travel to Level01).
 */
UCLASS()
class QIXX_API AMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMenuGameMode();

	/** Toggles the ready state of the given player (server side). */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	void PlayerPressedReady(APlayerController* PC);

	/** True when every connected player has pressed ready. */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	bool AreAllPlayersReady() const;

	/** Fills player names + ready states for the host lobby UI. */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	void GetLobbyPlayerList(TArray<FString>& OutNames, TArray<bool>& OutReady) const;

    /** Travels everyone to Level01 (host side trigger). */
    UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
    void StartGame();

    /**
     * Marks a given player as launched. If all players are launched, starts the game.
     * Server-side only.
     */
    void PlayerLaunched(APlayerController* PC);

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	/** Syncs the replicated lobby state to the game state (server side). */
	void UpdateLobbyState();

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> ReadyPlayers;
};
