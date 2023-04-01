// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StatusIconTypes.generated.h"

/**
 * 
 */

UENUM()
enum class EStatusIconQualityLevel : uint8
{
	Low,
	High //Performs clipping for progress instead of a transform scale (~2x as expensive, not able to be batched).
};

//A struct that represents an entity's processed status icon information.
//May want to consider this struct containing a copy of the fragment FIconSlateFragment and extra data needed instead of redeclaring all the values we want from the fragment.
USTRUCT()
struct SILENTWORLD_API FStatusIconEntry
{
	GENERATED_BODY()

	FStatusIconEntry() {}
	
	FStatusIconEntry(const FSlateBrush& InBarBrush, const FSlateBrush& InBackplateBrush, const float InProgress, const FVector2D& InScreenPosition, const float InScale,
		const int32 InZOrder = -1, const EStatusIconQualityLevel InQuality = EStatusIconQualityLevel::Low)
		: BarBrush(InBarBrush), BackplateBrush(InBackplateBrush), Progress(InProgress), ScreenPosition(InScreenPosition), Scale(InScale),
		ZOrder(InZOrder), Quality(InQuality) {}
	
	FSlateBrush BarBrush = FSlateBrush();
	FSlateBrush BackplateBrush = FSlateBrush();
	float Progress = -1.f;
	FVector2D ScreenPosition = FVector2D(-1.f);
	float Scale = 1.f;
	int32 ZOrder = -255;
	EStatusIconQualityLevel Quality = EStatusIconQualityLevel::Low;

	friend bool operator<(const FStatusIconEntry& A, const FStatusIconEntry& B)
	{
		return A.ZOrder < B.ZOrder;
	}
};