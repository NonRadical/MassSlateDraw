// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "RandomizedPatrolTrait.generated.h"

UENUM()
enum class EPatrolDirection : uint8
{
	Forward,
	Backward
};

USTRUCT()
struct SILENTWORLD_API FRandomizedPatrolFragment : public FMassFragment
{
	GENERATED_BODY()

	float PatrolLength = 0.f;
	float PatrolSpeed = 0.f;
	float PatrolAccel = 0.f;
	FVector PatrolStart = FVector(-UE_MAX_FLT);
	FVector PatrolEnd = FVector(-UE_MAX_FLT);
	EPatrolDirection Direction = EPatrolDirection::Forward;
	FVector MoveVelocity = FVector(0.f);
};

/**
 * 
 */
UCLASS()
class SILENTWORLD_API URandomizedPatrolTrait : public UMassEntityTraitBase
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	virtual void ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
protected:
	UPROPERTY(Category="Patrol", EditDefaultsOnly)
	float PatrolLength = 0.f;
	UPROPERTY(Category="Patrol", EditDefaultsOnly)
	float PatrolSpeed = 50.f;
	UPROPERTY(Category="Patrol", EditDefaultsOnly)
	float PatrolAccel = 10.f;
};

UCLASS()
class SILENTWORLD_API URandomizedPatrolInitializationProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

	URandomizedPatrolInitializationProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery PatrolRandomizerQuery;
};

UCLASS()
class SILENTWORLD_API URandomizedPatrolProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URandomizedPatrolProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery PatrolQuery;
};