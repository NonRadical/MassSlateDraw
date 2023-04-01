// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "StatusTrait.generated.h"

USTRUCT()
struct SILENTWORLD_API FStatusHealthFragment : public FMassFragment
{
	GENERATED_BODY()

	double Health = -1.0;
	double MaxHealth = -1.0;
};

/**
 * 
 */
UCLASS()
class SILENTWORLD_API UStatusTrait : public UMassEntityTraitBase
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
	UPROPERTY(Category="Status", EditDefaultsOnly)
	double MaxHealth = 0.0;
};
