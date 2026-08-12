#include "IsometricPlayerController.h"
#include "IsometricCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"

AIsometricPlayerController::AIsometricPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AIsometricPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	IsometricCharacter = Cast<AIsometricCharacter>(GetPawn());
}

void AIsometricPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIsometricPlayerController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIsometricPlayerController::Look);
	}
}

void AIsometricPlayerController::Move(const FInputActionValue& Value)
{
	if (!IsometricCharacter)
	{
		IsometricCharacter = Cast<AIsometricCharacter>(GetPawn());
		if (!IsometricCharacter)
		{
			return;
		}
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (MovementVector.X != 0.0f)
	{
		// Move forward/backward
		IsometricCharacter->AddMovementInput(IsometricCharacter->GetActorForwardVector(), MovementVector.X);
	}

	if (MovementVector.Y != 0.0f)
	{
		// Move right/left
		IsometricCharacter->AddMovementInput(IsometricCharacter->GetActorRightVector(), MovementVector.Y);
	}
}

void AIsometricPlayerController::Look(const FInputActionValue& Value)
{
	// Isometric view doesn't use look input in the traditional sense
	// This is here for future camera manipulation if needed
}
