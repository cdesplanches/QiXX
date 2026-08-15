#include "Menu/MenuGameMode.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include "Menu/MenuGameState.h"
#include "Menu/MenuPlayerController.h"

AMenuGameMode::AMenuGameMode()
{
	PlayerControllerClass = AMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	SpectatorClass = nullptr;
	GameStateClass = AMenuGameState::StaticClass();
}

void AMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// A freshly connected player is not ready
	ReadyPlayers.Remove(NewPlayer);
	UpdateLobbyState();
}

void AMenuGameMode::PlayerPressedReady(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	if (ReadyPlayers.Contains(PC))
	{
		ReadyPlayers.Remove(PC);
	}
	else
	{
		ReadyPlayers.AddUnique(PC);
	}

	UpdateLobbyState();
}

bool AMenuGameMode::AreAllPlayersReady() const
{
    return GetGameState<AMenuGameState>() ? GetGameState<AMenuGameState>()->AreAllPlayersReady() : false;
}

// New: record that a player has launched and start the game when everyone is ready
void AMenuGameMode::PlayerLaunched(APlayerController* PC)
{
    if (!PC)
    {
        return;
    }
    if (!ReadyPlayers.Contains(PC))
    {
        ReadyPlayers.AddUnique(PC);
    }
    UpdateLobbyState();
    if (AreAllPlayersReady())
    {
        StartGame();
    }
}

void AMenuGameMode::GetLobbyPlayerList(TArray<FString>& OutNames, TArray<bool>& OutReady) const
{
	if (GetGameState<AMenuGameState>())
	{
		GetGameState<AMenuGameState>()->GetLobbyPlayerList(OutNames, OutReady);
	}
	else
	{
		OutNames.Reset();
		OutReady.Reset();
	}
}

void AMenuGameMode::StartGame()
{
	// Preserve the current listen port so reconnecting clients can rejoin (PIE offsets the port),
	// but explicitly set the gameplay GameMode to avoid inheriting the lobby's "game=" URL option.
	GetWorld()->ServerTravel(
			AMenuPlayerController::MakeServerTravelURL(
					GetWorld(),
					TEXT("/Game/Level01/Lvl_01?listen?game=/Game/QiXX/Core/BP_QiXXGameMode.BP_QiXXGameMode_C")),
		false);
}

void AMenuGameMode::UpdateLobbyState()
{
	TArray<APlayerController*> ReadyControllers;
	ReadyControllers.Reserve(ReadyPlayers.Num());
	for (const TObjectPtr<APlayerController>& PC : ReadyPlayers)
	{
		ReadyControllers.Add(PC);
	}

	if (AMenuGameState* MenuGameState = GetGameState<AMenuGameState>())
	{
		MenuGameState->SetReadyPlayers(ReadyControllers);
	}
}
