// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassObserverProcessor.h"
#include "StatusIconProcessor.generated.h"

//Processor responsible for taking all status icons and projecting their world position onto the current screen.
UCLASS()
class SILENTWORLD_API UStatusIconProcessor : public UMassProcessor
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery StatusIconQuery;
};


//Simple observer processor to randomize health.
UCLASS()
class SILENTWORLD_API UHealthRandomizerProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

	UHealthRandomizerProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery HealthRandomizerQuery;
};