// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/StatusIconLayer.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Mass/Trait/StatusIconTrait.h"
#include "Mass/Subsystem/StatusIconSubsystem.h"

namespace SW::StatusIconProcessor
{
	static bool bPerformProgressBarClipping = true;
	static FAutoConsoleVariableRef CVarPerformProgressBarClipping(
		TEXT("SW.IconProcessor.PerformProgressBarClipping"),
		bPerformProgressBarClipping,
		TEXT("If true, high quality status icon progress bars will be clipped instead of scaled."),
		ECVF_Default);
}

FName FStatusIconScreenLayer::LayerName = "";

void SStatusIconScreenLayer::Construct(const FArguments& InArgs, const FLocalPlayerContext& InPlayerContext)
{
	PlayerContext = InPlayerContext;
	bCanSupportFocus = false;
}

FORCEINLINE void DrawStatusIcon(const FIconStateFragment& IconState, const FIconSlateFragment& IconSlate, const FIconProgressFragment& IconProgress,
	const float ViewportScale, FSlateClippingZone& ClippingZone, FPaintGeometry& PaintGeometry, FSlateWindowElementList& OutDrawElements,
	const FLinearColor& MasterTint, const int32 LayerId, const bool bCanUseProgressClip)
{
	const float EntryDrawScale = ViewportScale * (IconState.DrawScale != -1.f ? IconState.DrawScale : 1.f);
	const FVector2D BackplateSize = EntryDrawScale * (IconSlate.BackplateBrush.ImageSize / PaintGeometry.GetLocalSize());
	const FVector2D BackplateDrawPosition = IconState.ScreenPosition - (IconSlate.BackplateBrush.ImageSize * 0.5f * EntryDrawScale);
	const FVector2D RoundedBackplateDrawPosition = FVector2D(FMath::RoundToInt(BackplateDrawPosition.X), FMath::RoundToInt(BackplateDrawPosition.Y));
	PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BackplateSize), RoundedBackplateDrawPosition));
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &IconSlate.BackplateBrush, ESlateDrawEffect::None, IconSlate.BackplateBrush.TintColor.GetSpecifiedColor() * MasterTint);
	
	if(IconProgress.Progress == 0.f)
	{
		return;
	}
	
	const FVector2D BarDrawPosition = IconState.ScreenPosition - (IconSlate.BarBrush.ImageSize * 0.5f * EntryDrawScale);
	const FVector2D RoundedBarDrawPosition = FVector2D(FMath::RoundToInt(BarDrawPosition.X), FMath::RoundToInt(BarDrawPosition.Y));
	
	const FVector2D BarScale = EntryDrawScale * (IconSlate.BarBrush.ImageSize / PaintGeometry.GetLocalSize());
	
	if(IconProgress.Progress >= 1.f)
	{
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BarScale), RoundedBarDrawPosition));
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &IconSlate.BarBrush, ESlateDrawEffect::None, IconSlate.BarBrush.TintColor.GetSpecifiedColor() * MasterTint);
		return;
	}
	
	const bool bIconShouldPerformProgressClipping = bCanUseProgressClip && IconSlate.QualityLevel == EStatusIconQualityLevel::High;

	/*It's important to note that using draw clipping has two very important drawbacks;
	*   Elements drawn with arbitrary draw clipping cannot be batched with other elements and are much more expensive.
	*   Due to not being batched, they are drawn individually after the initial non-clipping draw.
    *		(This means clipped progress bars will always draw on top of the batched elements and their ordering may cause clipped elements to draw on top of other elements when not desired.)
	*/
	if(bIconShouldPerformProgressClipping)
	{
		const float BarSizeY = FMath::CeilToFloat(IconSlate.BarBrush.ImageSize.Y * EntryDrawScale);
		ClippingZone.TopLeft = FVector2f(RoundedBarDrawPosition);
		ClippingZone.BottomLeft = ClippingZone.TopLeft;
		ClippingZone.BottomLeft.Y += BarSizeY;
	
		ClippingZone.TopRight = FVector2f(RoundedBarDrawPosition);
		ClippingZone.TopRight.X += (IconSlate.BarBrush.ImageSize.X * IconProgress.Progress * EntryDrawScale);
		ClippingZone.BottomRight = ClippingZone.TopRight;
		ClippingZone.BottomRight.Y += BarSizeY;
		OutDrawElements.PushClip(ClippingZone);		
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BarScale), RoundedBarDrawPosition));
	}
	else
	{
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BarScale.X * IconProgress.Progress, BarScale.Y), RoundedBarDrawPosition));
	}

	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &IconSlate.BarBrush, ESlateDrawEffect::None, IconSlate.BarBrush.TintColor.GetSpecifiedColor() * MasterTint);

	if(bIconShouldPerformProgressClipping)
	{
		OutDrawElements.PopClip();
	}	
}

int32 SStatusIconScreenLayer::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_StatusIcon_PaintIcons)
	
	const UWorld* World = PlayerContext.GetWorld();
	if(!World)
	{
		return LayerId;
	}
	
	const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);

	if(!bIsEnabled)
	{
		return LayerId;
	}

	FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry();
	const FLinearColor MasterTint = FLinearColor::White;

	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(World);
	if (!EntitySubsystem)
	{
		return LayerId;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	
	UGameViewportClient* ViewportClient = World->GetGameViewport();
	if(!ViewportClient)
	{
		return LayerId;
	}
	
	const float ViewportScale = FMath::Max((FMath::RoundToFloat(2.f * UWidgetLayoutLibrary::GetViewportScale(ViewportClient)) * 0.5f) - 0.5f, 0.5f);
	const bool bCanUseProgressClip = SW::StatusIconProcessor::bPerformProgressBarClipping;

	FMassExecutionContext ExecutionContext;
	FMassEntityQuery Query;
	Query.AddRequirement<FIconStateFragment>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FIconProgressFragment>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FIconSlateFragment>(EMassFragmentAccess::ReadOnly);
	Query.ForEachEntityChunk(EntityManager, ExecutionContext, [&OutDrawElements, PaintGeometry, MyCullingRect, ViewportScale, LayerId, bCanUseProgressClip, MasterTint](FMassExecutionContext& Context)
	{
		const TConstArrayView<FIconStateFragment> PositionDataList = Context.GetFragmentView<FIconStateFragment>();
		const TConstArrayView<FIconProgressFragment> IconProgressList = Context.GetFragmentView<FIconProgressFragment>();
		const TConstArrayView<FIconSlateFragment> StatusIconList = Context.GetFragmentView<FIconSlateFragment>();

		FPaintGeometry CurrentPaintGeometry = FPaintGeometry(PaintGeometry);
		FSlateClippingZone ClippingZone(MyCullingRect);

		const int32 NumEntities = Context.GetNumEntities();
		for (int32 Index = NumEntities - 1; Index >= 0; Index--)
		{
			const FIconStateFragment& IconState = PositionDataList[Index];

			if(IconState.ScreenPosition == FVector2D(-UE_MAX_FLT))
			{
				continue;
			}
		
			const FIconProgressFragment& IconProgress = IconProgressList[Index];
			
			const FIconSlateFragment& IconSlate = StatusIconList[Index];
		
			DrawStatusIcon(IconState, IconSlate, IconProgress, ViewportScale, ClippingZone, CurrentPaintGeometry, OutDrawElements, MasterTint, LayerId, bCanUseProgressClip);
		}
	});

	return LayerId;
}

TSharedRef<SWidget> FStatusIconScreenLayer::AsWidget()
{
	if (TSharedPtr<SStatusIconScreenLayer> ScreenLayer = ScreenLayerPtr.Pin())
	{
		return ScreenLayer.ToSharedRef();
	}

	TSharedRef<SStatusIconScreenLayer> NewScreenLayer = SNew(SStatusIconScreenLayer, OwningPlayer);
	ScreenLayerPtr = NewScreenLayer;
	return NewScreenLayer;
}