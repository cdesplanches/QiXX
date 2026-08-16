#include "Menu/MenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"

#include "QiXX.h"
#include "Isometric/IsometricFunctionLibrary.h"
#include "Menu/MenuGameMode.h"
#include "Menu/MenuGameState.h"

AMenuPlayerController::AMenuPlayerController()
{
}

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Menus use UI-only input mode with a visible mouse cursor
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.Contains(TEXT("Lobby")))
	{
		ShowLobby();
	}
	else
	{
		ShowSplashAndMainMenu();
	}
}

void AMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SplashCheckTimerHandle);
	GetWorldTimerManager().ClearTimer(LobbyRefreshTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AMenuPlayerController::ShowSplashAndMainMenu()
{
	if (SplashWidgetClass)
	{
		ShowWidget(SplashWidgetClass, 10);
		GetWorldTimerManager().SetTimer(SplashCheckTimerHandle, this, &AMenuPlayerController::CheckSplash, 0.5f, true);
	}
	else if (MainMenuWidgetClass)
	{
		ShowWidget(MainMenuWidgetClass);
		ApplyMenuRoleUI();
	}
}

void AMenuPlayerController::CheckSplash()
{
	if (UIsometricFunctionLibrary::IsChunkAvailable(1001))
	{
		GetWorldTimerManager().ClearTimer(SplashCheckTimerHandle);
		HideActiveMenuWidget();
		if (MainMenuWidgetClass)
		{
			ShowWidget(MainMenuWidgetClass);
			ApplyMenuRoleUI();
		}
	}
}

void AMenuPlayerController::ShowLobby()
{
	if (LobbyWidgetClass)
	{
		ShowWidget(LobbyWidgetClass);
		GetWorldTimerManager().SetTimer(LobbyRefreshTimerHandle, this, &AMenuPlayerController::RefreshLobbyStatus, 0.5f, true);
	}
}

void AMenuPlayerController::RefreshLobbyStatus()
{
	if (!ActiveMenuWidget)
	{
		return;
	}

	if (UTextBlock* StatusText = Cast<UTextBlock>(ActiveMenuWidget->GetWidgetFromName(TEXT("StatusText"))))
	{
		StatusText->SetText(FText::FromString(GetLobbyStatus()));
	}
}

void AMenuPlayerController::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 InZOrder)
{
    // Fix: ensure we only create widgets for the local player controller
    if (!IsLocalController())
    {
        return;
    }
    if (!WidgetClass)
    {
        return;
    }

    UUserWidget* Widget = CreateWidget<UUserWidget>(this, WidgetClass);
    if (Widget)
    {
        Widget->AddToViewport(InZOrder);
        ActiveMenuWidget = Widget;
    }
}

void AMenuPlayerController::HideActiveMenuWidget()
{
	if (ActiveMenuWidget)
	{
		ActiveMenuWidget->RemoveFromParent();
		ActiveMenuWidget = nullptr;
	}
}

FString AMenuPlayerController::MakeServerTravelURL(const UWorld* World, const FString& MapPath)
{
	if (!World)
	{
		return MapPath;
	}

	// UE PIE can start a listen server on a port that does not match the default
	// used when doing ServerTravel("... ?listen"). ServerTravel doesn't reliably
	// preserve the PIE port, but UGameInstance::EnableListenServer uses
	// WorldContext->LastURL.Port.
	//
	// So: copy the currently bound local port from the net driver into
	// WorldContext->LastURL.Port before traveling.
	UNetDriver* NetDriver = const_cast<UWorld*>(World)->GetNetDriver();
	if (NetDriver)
	{
		if (const TSharedPtr<const FInternetAddr> LocalAddr = NetDriver->GetLocalAddr(); LocalAddr.IsValid())
		{
			const int32 ListenPort = LocalAddr->GetPort();
			if (ListenPort > 0 && GEngine)
			{
				if (FWorldContext* WC = GEngine->GetWorldContextFromWorld(const_cast<UWorld*>(World)))
				{
					WC->LastURL.Port = ListenPort;
				}
			}
		}
	}

	// Keep travel URL as a map URL (no host:port) because UE blocks such FURLs.
	return MapPath;
}

void AMenuPlayerController::HostLobbyGame()
{
	if (!IsLocalController() || !GetWorld())
	{
		return;
	}

	// OpenLevel on a listen server would only travel the server and drop clients.
	// ServerTravel carries every connected client to the lobby automatically.
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		GetWorld()->ServerTravel(MakeServerTravelURL(GetWorld(), TEXT("/Game/QiXX/Maps/Lvl_Lobby?listen")), false);
	}
}

void AMenuPlayerController::JoinLobbyGame(const FString& Address)
{
	if (!IsLocalController() || !GetWorld())
	{
		return;
	}

	// Joining only makes sense when not already hosting a session.
	const ENetMode NetMode = GetWorld()->GetNetMode();
	if (NetMode == NM_ListenServer || NetMode == NM_DedicatedServer)
	{
		return;
	}

	if (Address.IsEmpty())
	{
		UE_LOG(LogQiXX, Warning, TEXT("JoinLobbyGame: empty address"));
		return;
	}

	ClientTravel(Address, TRAVEL_Absolute);
}

void AMenuPlayerController::ServerToggleReady_Implementation()
{
	if (AMenuGameMode* MenuGameMode = GetWorld()->GetAuthGameMode<AMenuGameMode>())
	{
		MenuGameMode->PlayerPressedReady(this);
	}
}

void AMenuPlayerController::ServerStartGame_Implementation()
{
	if (!IsLocalController())
	{
		return; // Only the host may start the game
	}

	if (AMenuGameMode* MenuGameMode = GetWorld()->GetAuthGameMode<AMenuGameMode>())
	{
		if (MenuGameMode->AreAllPlayersReady())
		{
			MenuGameMode->StartGame();
		}
	}
}

bool AMenuPlayerController::AreAllPlayersReady() const
{
	if (const AMenuGameState* MenuGameState = GetWorld() ? GetWorld()->GetGameState<AMenuGameState>() : nullptr)
	{
		return MenuGameState->AreAllPlayersReady();
	}
	return false;
}

void AMenuPlayerController::GetLobbyPlayerList(TArray<FString>& OutNames, TArray<bool>& OutReady) const
{
	OutNames.Reset();
	OutReady.Reset();

	if (const AMenuGameState* MenuGameState = GetWorld() ? GetWorld()->GetGameState<AMenuGameState>() : nullptr)
	{
		MenuGameState->GetLobbyPlayerList(OutNames, OutReady);
	}
}

bool AMenuPlayerController::IsHost() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetNetMode() != NM_Client;
	}
	return true;
}

FString AMenuPlayerController::GetHostAddress() const
{
	if (const UWorld* World = GetWorld())
	{
		if (UNetDriver* NetDriver = World->GetNetDriver())
		{
			const TSharedPtr<const FInternetAddr> LocalAddr = NetDriver->GetLocalAddr();
			if (LocalAddr.IsValid())
			{
				return FString::Printf(TEXT("%s:%d"), *LocalAddr->ToString(false), LocalAddr->GetPort());
			}
		}
	}

	// Fallback: machine primary IP + default listen port.
	FString PrimaryIP = TEXT("127.0.0.1");
	if (ISocketSubsystem* Sockets = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
	{
		bool bCanBind = false;
		const TSharedPtr<FInternetAddr> HostAddr = Sockets->GetLocalHostAddr(*GLog, bCanBind);
		if (HostAddr.IsValid())
		{
			PrimaryIP = HostAddr->ToString(false);
		}
	}

	return FString::Printf(TEXT("%s:7777"), *PrimaryIP);
}

void AMenuPlayerController::ApplyMenuRoleUI()
{
	if (!ActiveMenuWidget)
	{
		return;
	}

	const bool bHost = IsHost();

	if (UWidget* CreateButton = ActiveMenuWidget->GetWidgetFromName(TEXT("CreateButton")))
	{
		CreateButton->SetVisibility(bHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (UWidget* IpInputWidget = ActiveMenuWidget->GetWidgetFromName(TEXT("IpInput")))
	{
		if (UEditableText* IpInput = Cast<UEditableText>(IpInputWidget))
		{
			IpInput->SetText(FText::FromString(GetHostAddress()));
		}
	}
}

FString AMenuPlayerController::GetLobbyStatus() const
{
	TArray<FString> Names;
	TArray<bool> Ready;
	GetLobbyPlayerList(Names, Ready);

	FString Status;
	for (int32 i = 0; i < Names.Num(); ++i)
	{
		if (i > 0)
		{
			Status += TEXT("\n");
		}
		Status += Names[i];
		Status += Ready[i] ? TEXT(" [pret]") : TEXT(" [pas pret]");
	}

	// Host exposes the connection string so friends know what to type to join.
	if (IsHost())
	{
		if (!Status.IsEmpty())
		{
			Status += TEXT("\n");
		}
		Status += FString::Printf(TEXT("IP: %s"), *GetHostAddress());
	}

	return Status;
}

// New: Launch flow (UI calls -> RPCs -> GameMode integration)
void AMenuPlayerController::LaunchGame()
{
    if (!IsLocalController())
    {
        return;
    }
    if (LoadingWidgetClass)
    {
        ShowWidget(LoadingWidgetClass, 100);
    }
    ServerLaunchGame();
}

void AMenuPlayerController::LeaveToMainMenu()
{
    if (!IsLocalController()) return;
    if (GetWorld())
    {
        if (GetWorld()->GetNetMode() != NM_Client)
        {
            GetWorld()->ServerTravel(MakeServerTravelURL(GetWorld(), TEXT("/Game/QiXX/Maps/Lvl_MainMenu?listen")), false);
        }
        else
        {
            ClientTravel(TEXT("/Game/QiXX/Maps/Lvl_MainMenu"), TRAVEL_Absolute);
        }
    }
}

bool AMenuPlayerController::ServerLaunchGame_Validate()
{
    return true;
}

void AMenuPlayerController::ServerLaunchGame_Implementation()
{
    if (AMenuGameMode* MenuGameMode = GetWorld()->GetAuthGameMode<AMenuGameMode>())
    {
        MenuGameMode->PlayerLaunched(this);
    }
}
