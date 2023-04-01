// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/Trait/StatusIconTrait.h"
#include "Mass/Trait/StatusTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "VisualLogger/VisualLogger.h"

DEFINE_LOG_CATEGORY(LogStatus);

UStatusIconTrait::UStatusIconTrait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

inline FVector2D MaxVector2D(const FVector2D& A, const FVector2D& B)
{
	return FVector2D(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
}

void UStatusIconTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	if (World.IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	FIconStateFragment& StateFragment = BuildContext.AddFragment_GetRef<FIconStateFragment>();
	StateFragment.WorldOffset = WorldOffset;
	StateFragment.ExtentHalfSize = MaxVector2D(BarBrush.ImageSize, BackplateBrush.ImageSize); //Cache the base extent here so we don't need to pull in all slate data during projection time.
	StateFragment.DrawScale = bShouldScale ? 0.f : -1.f;
	
	FIconSlateFragment& SlateFragment = BuildContext.AddFragment_GetRef<FIconSlateFragment>();
	SlateFragment.BarBrush = BarBrush;
	SlateFragment.BackplateBrush = BackplateBrush;
	SlateFragment.QualityLevel = QualityLevel;
	
	FIconProgressFragment& IconProgressFragment = BuildContext.AddFragment_GetRef<FIconProgressFragment>();
	IconProgressFragment.TrackedValue = TrackedValue;

	if(TrackedValue == EIconProgressValueSelector::Health)
	{
		BuildContext.RequireFragment<FStatusHealthFragment>();
	}
}

void UStatusIconTrait::ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	if (World.IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	FIconProgressFragment& IconProgressFragment = BuildContext.AddFragment_GetRef<FIconProgressFragment>();
	
	if(TrackedValue == EIconProgressValueSelector::Health && !BuildContext.HasFragment<FStatusHealthFragment>())
	{
		UE_VLOG(&World, LogStatus, Error, TEXT("Failed to find Status Health Fragment but Icon Progress Value Selector contained EIconProgressValueSelector::Health."));
		IconProgressFragment.TrackedValue = EIconProgressValueSelector::Invalid;
	}
}