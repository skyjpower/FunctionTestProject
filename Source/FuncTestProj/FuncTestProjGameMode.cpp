// Copyright Epic Games, Inc. All Rights Reserved.

#include "FuncTestProjGameMode.h"
#include "FuncTestProjCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFuncTestProjGameMode::AFuncTestProjGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
