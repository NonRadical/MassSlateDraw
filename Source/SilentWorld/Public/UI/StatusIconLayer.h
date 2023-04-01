#pragma once

#include "Slate/SGameLayerManager.h"
#include "Widgets/SWidget.h"
#include "Engine/LocalPlayer.h"
#include "MassEntityQuery.h"

/*
 * Similar to how UWidgetComponent does its fullscreen slate container for drawing all the components' UWidgets.
 * The main difference is that we're not managing any canvas slots or anything.
 * This is because we won't be drawing some list of SWidgets and instead manually invoking draw elements ourselves.
 */
class SStatusIconScreenLayer : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SStatusIconScreenLayer)
	{
		_Visibility = EVisibility::HitTestInvisible;
	}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InPlayerContext);
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(0, 0); }
	
protected:
	FLocalPlayerContext PlayerContext;
};

class FStatusIconScreenLayer : public IGameLayer
{
public:
	FStatusIconScreenLayer(const FLocalPlayerContext& PlayerContext)
	{
		OwningPlayer = PlayerContext;
		ScreenLayerPtr = nullptr;
	}
	
	virtual ~FStatusIconScreenLayer()
	{
		// empty virtual destructor to help clang warning
	}

	virtual TSharedRef<SWidget> AsWidget() override;
	
	static FName LayerName;
	
private:
	FLocalPlayerContext OwningPlayer;
	TWeakPtr<SStatusIconScreenLayer> ScreenLayerPtr;
};
