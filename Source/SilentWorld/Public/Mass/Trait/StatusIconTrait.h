// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "MassProcessor.h"
#include "Styling/SlateBrush.h"
#include "Mass/StatusIconTypes.h"
#include "StatusIconTrait.generated.h"

SILENTWORLD_API DECLARE_LOG_CATEGORY_EXTERN(LogStatus, Log, All);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EIconProgressValueSelector : uint8
{
	Invalid = 0 UMETA(Hidden),
	Health = 1 << 0,
	Energy = 1 << 1,
	Progress = 1 << 2,
	Special = 1 << 3
};
ENUM_CLASS_FLAGS(EIconProgressValueSelector);

inline int32 operator& (const EIconProgressValueSelector LHS, const int32 RHS) { return (static_cast<int32>(LHS) & RHS); }
inline int32 operator& (const int32 LHS, const EIconProgressValueSelector RHS) { return (LHS & static_cast<int32>(RHS)); }
inline uint8 operator& (const EIconProgressValueSelector LHS, const uint8 RHS) { return (static_cast<uint8>(LHS) & RHS); }
inline uint8 operator& (const uint8 LHS, const EIconProgressValueSelector RHS) { return (LHS & static_cast<uint8>(RHS)); }

//Fragment containing the draw state for a given entity.
USTRUCT()
struct SILENTWORLD_API FIconStateFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector WorldOffset = FVector(0.f);
	FVector2D ExtentHalfSize = FVector2D(0.f);
	FVector2D ScreenPosition = FVector2D(-UE_MAX_FLT);
	
	float DrawScale = -1.f; //If DrawScale is -1.f, it implies we don't want to perform scaling (UStatusIconTrait::bShouldScale will set this to 0.f by default if true).
};

//Fragment representing progress data/source for a given entity.
USTRUCT()
struct SILENTWORLD_API FIconProgressFragment : public FMassFragment
{
	GENERATED_BODY()

	EIconProgressValueSelector TrackedValue = EIconProgressValueSelector::Invalid;
	float Progress = -1.f;
};


//Fragment containing the slate data for a given entity. Only used at draw time to reduce iteration cost at projection time.
USTRUCT()
struct SILENTWORLD_API FIconSlateFragment : public FMassFragment
{
	GENERATED_BODY()

	FSlateBrush BackplateBrush = FSlateBrush();
	FSlateBrush BarBrush = FSlateBrush();
	int32 ZOrder = -255;
	EStatusIconQualityLevel QualityLevel = EStatusIconQualityLevel::Low;
};

/**
 * 
 */
UCLASS()
class SILENTWORLD_API UStatusIconTrait : public UMassEntityTraitBase
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	virtual void ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
protected:
	UPROPERTY(Category="StatusIcon", EditDefaultsOnly)
	FSlateBrush BarBrush = FSlateBrush();
	UPROPERTY(Category="StatusIcon", EditDefaultsOnly)
	FSlateBrush BackplateBrush = FSlateBrush();

	UPROPERTY(Category="StatusIcon", EditDefaultsOnly)
	EIconProgressValueSelector TrackedValue = EIconProgressValueSelector::Invalid;
	UPROPERTY(Category="StatusIcon", EditDefaultsOnly)
	FVector WorldOffset = FVector(0.f);
	UPROPERTY(Category="StatusIcon", EditDefaultsOnly)
	bool bShouldScale = true;
	UPROPERTY(Category="StatusIcon", EditDefaultsOnly)
	EStatusIconQualityLevel QualityLevel = EStatusIconQualityLevel::Low;
};
