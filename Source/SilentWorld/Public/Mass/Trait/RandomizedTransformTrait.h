// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "MassObserverProcessor.h"
#include "RandomizedTransformTrait.generated.h"

//This is a pretty big fragment... I think this is the wrong way of doing this... Probably belongs elsewhere.
USTRUCT()
struct SILENTWORLD_API FRandomizedTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	FRotator RotationRandomizationMin = FRotator(0.f);
	FRotator RotationRandomizationMax = FRotator(0.f);
	FVector ScaleRandomizationMin = FVector(1.f);
	FVector ScaleRandomizationMax = FVector(1.f);
	FVector TranslationRandomizationMin = FVector(1.f);
	FVector TranslationRandomizationMax = FVector(1.f);
};

/**
 * 
 */
UCLASS()
class SILENTWORLD_API URandomizedTransformTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
	UPROPERTY(Category="Randomization", EditDefaultsOnly)
	bool bApplyRotation = false;
	UPROPERTY(Category="Randomization", EditDefaultsOnly, meta = (EditCondition = "bApplyRotation"))
	FRotator RotationRandomizationMin = FRotator(0.f);
	UPROPERTY(Category="Randomization", EditDefaultsOnly, meta = (EditCondition = "bApplyRotation"))
	FRotator RotationRandomizationMax = FRotator(0.f);
	UPROPERTY(Category="Randomization", EditDefaultsOnly)
	bool bApplyScale = false;
	UPROPERTY(Category="Randomization", EditDefaultsOnly, meta = (EditCondition = "bApplyScale"))
	FVector ScaleRandomizationMin = FVector(1.f);
	UPROPERTY(Category="Randomization", EditDefaultsOnly, meta = (EditCondition = "bApplyScale"))
	FVector ScaleRandomizationMax = FVector(1.f);
	
	UPROPERTY(Category="Randomization", EditDefaultsOnly)
	bool bApplyTranslation = false;
	UPROPERTY(Category="Randomization", EditDefaultsOnly, meta = (EditCondition = "bApplyTranslation"))
	FVector TranslationRandomizationMin = FVector(0.f);
	UPROPERTY(Category="Randomization", EditDefaultsOnly, meta = (EditCondition = "bApplyTranslation"))
	FVector TranslationRandomizationMax = FVector(0.f);
};

UCLASS()
class SILENTWORLD_API UTransformRandomizerProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

	UTransformRandomizerProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery TransformRandomizerQuery;
};