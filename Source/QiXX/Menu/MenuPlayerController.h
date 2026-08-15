#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

class UUserWidget;

/**
 * Player controller used in menu and lobby maps.
 * Handles UI-only input, widget display, and lobby RPCs (ready / start).
 */
UCLASS()
class QIXX_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMenuPlayerController();

	/** Host only: travel the server to the lobby so every connected client follows. */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	void HostLobbyGame();

	/** Client only: travel to a remote lobby listen server at the given address. */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	void JoinLobbyGame(const FString& Address);

    /** Builds a ServerTravel URL that preserves the current listen port. */
    static FString MakeServerTravelURL(const UWorld* World, const FString& MapPath);

    /** Client -> Server: toggle this player's ready state in the lobby. (to be superseded by Launch flow) */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "QiXX|Menu")
    void ServerToggleReady();

    /** Client -> Server: launch the game from the lobby (new flow). */
    UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
    void LaunchGame();

    /** Client -> Server: leave lobby to return to main menu. */
    UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
    void LeaveToMainMenu();

    /** Loading overlay widget class used when a player launches. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QiXX|UI")
    TSubclassOf<UUserWidget> LoadingWidgetClass;

    /** Server RPC to start game after a player launched. */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "QiXX|Menu")
    void ServerLaunchGame();

	/** Client -> Server: host requests to start the game from the lobby. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "QiXX|Menu")
	void ServerStartGame();

	/** True when every connected player has pressed ready. */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	bool AreAllPlayersReady() const;

	/** Fills lobby display data: player names + ready states. */
	UFUNCTION(BlueprintCallable, Category = "QiXX|Menu")
	void GetLobbyPlayerList(TArray<FString>& OutNames, TArray<bool>& OutReady) const;

	/** Formatted lobby status string, e.g. "2/2 pret" or per-player lines. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QiXX|Menu")
	FString GetLobbyStatus() const;

	/** Widget blueprint shown on the splash screen, set in the child BP class defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QiXX|UI")
	TSubclassOf<UUserWidget> SplashWidgetClass;

	/** Widget blueprint shown in the main menu, set in the child BP class defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QiXX|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	/** Widget blueprint shown in the lobby, set in the child BP class defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QiXX|UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ServerToggleReady_Implementation();
	void ServerStartGame_Implementation();
	bool ServerLaunchGame_Validate();
	void ServerLaunchGame_Implementation();

	void ShowSplashAndMainMenu();
	void ShowLobby();
	void CheckSplash();
	void RefreshLobbyStatus();
	void ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 InZOrder = 0);
	void HideActiveMenuWidget();

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> SplashWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveMenuWidget;

	FTimerHandle SplashCheckTimerHandle;
	FTimerHandle LobbyRefreshTimerHandle;
};
