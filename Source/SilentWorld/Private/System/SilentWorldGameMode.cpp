// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/SilentWorldGameMode.h"
#include "Player/SilentWorldPlayerController.h"
#include "Character/SilentWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASilentWorldGameMode::ASilentWorldGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASilentWorldPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Player/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Player/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}