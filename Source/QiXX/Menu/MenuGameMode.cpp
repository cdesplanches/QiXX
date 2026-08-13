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
	// Preserve the current listen port so reconnecting clients can rejoin (PIE offsets the port).
	GetWorld()->ServerTravel(AMenuPlayerController::MakeServerTravelURL(GetWorld(), TEXT("/Game/QiXX/Maps/Lvl_01?listen")), false);
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
