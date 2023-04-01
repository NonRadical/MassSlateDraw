// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/SilentWorldHUD.h"
#include "GameFramework/PlayerController.h"
#include "UI/StatusIconLayer.h"
#include "Slate/SGameLayerManager.h"

void ASilentWorldHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(StatusIconScreenLayer.IsValid())
	{
		return;
	}
	
	ULocalPlayer* LocalPlayer = GetOwningPlayerController() ? GetOwningPlayerController()->GetLocalPlayer() : nullptr;
	if(!LocalPlayer || !LocalPlayer->ViewportClient)
	{
		return;
	}

	TSharedPtr<IGameLayerManager> LayerManager = LocalPlayer->ViewportClient->GetGameLayerManager();

	if (LayerManager.IsValid())
	{
		TSharedPtr<IGameLayer> Layer = LayerManager->FindLayerForPlayer(LocalPlayer, FStatusIconScreenLayer::LayerName);
		if (!Layer.IsValid())
		{
			TSharedRef<FStatusIconScreenLayer> NewScreenLayer = MakeShareable(new FStatusIconScreenLayer(FLocalPlayerContext(LocalPlayer, GetWorld())));
			LayerManager->AddLayerForPlayer(LocalPlayer, FStatusIconScreenLayer::LayerName, NewScreenLayer, -100);
			StatusIconScreenLayer = NewScreenLayer;
		}
	}
}