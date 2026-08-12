#include "Menu/MenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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

	return Status;
}
