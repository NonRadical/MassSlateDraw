// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Mass/StatusIconTypes.h"
#include "StatusIconSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SILENTWORLD_API UStatusIconSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	const TArray<FStatusIconEntry>& GetStatusIconList() const { return StatusIconList; }
	TArray<FStatusIconEntry>& GetStatusIconList() { return StatusIconList; }
	
protected:
	UPROPERTY()
	TArray<FStatusIconEntry> StatusIconList = TArray<FStatusIconEntry>();
};
