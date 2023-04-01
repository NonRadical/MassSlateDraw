// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SilentWorldHUD.generated.h"

/**
 * 
 */
UCLASS()
class SILENTWORLD_API ASilentWorldHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;

protected:
	TSharedPtr<class FStatusIconScreenLayer> StatusIconScreenLayer = nullptr;
};
