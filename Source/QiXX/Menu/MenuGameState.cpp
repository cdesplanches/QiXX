#include "Menu/MenuGameState.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

AMenuGameState::AMenuGameState()
{
}

void AMenuGameState::OnRep_ReadyPlayerNames()
{
}

void AMenuGameState::SetReadyPlayers(const TArray<APlayerController*>& ReadyPlayers)
{
	ReadyPlayerNames.Reset();
	for (const APlayerController* PC : ReadyPlayers)
	{
		if (PC && PC->PlayerState)
		{
			ReadyPlayerNames.AddUnique(PC->PlayerState->GetPlayerName());
		}
	}
}

bool AMenuGameState::AreAllPlayersReady() const
{
	int32 TotalPlayers = 0;

	for (const TObjectPtr<APlayerState>& PS : PlayerArray)
	{
		if (PS)
		{
			++TotalPlayers;
			if (!ReadyPlayerNames.Contains(PS->GetPlayerName()))
			{
				return false;
			}
		}
	}

	return TotalPlayers >= 2;
}

void AMenuGameState::GetLobbyPlayerList(TArray<FString>& OutNames, TArray<bool>& OutReady) const
{
	OutNames.Reset();
	OutReady.Reset();

	for (const TObjectPtr<APlayerState>& PS : PlayerArray)
	{
		if (PS)
		{
			OutNames.Add(PS->GetPlayerName());
			OutReady.Add(ReadyPlayerNames.Contains(PS->GetPlayerName()));
		}
	}
}

void AMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMenuGameState, ReadyPlayerNames);
}
