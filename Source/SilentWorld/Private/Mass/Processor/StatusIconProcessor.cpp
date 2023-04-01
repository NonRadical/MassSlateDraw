// Copyright Epic Games, Inc. All Rights Reserved.


#include "Mass/Processor/StatusIconProcessor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "MassCommonFragments.h"
#include "Mass/Trait/StatusTrait.h"
#include "Mass/Trait/StatusIconTrait.h"
#include "Mass/Subsystem/StatusIconSubsystem.h"
#include "UI/StatusIconLayer.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

namespace SW::StatusIconProcessor
{
	static bool bPerformPrecullingInProcessor = true;
	static FAutoConsoleVariableRef CVarPerformPrecullingInProcessor(
		TEXT("SW.IconProcessor.PerformPreculling"),
		bPerformPrecullingInProcessor,
		TEXT("If true, the icon mass processor will attempt to perform culling before it adds an entry to the status icon list."),
		ECVF_Default);

	static bool bPerformTagBasedFiltering = true;
	static FAutoConsoleVariableRef CVarHPerformTagBasedFiltering(
		TEXT("SW.IconProcessor.PerformTagBasedFiltering"),
		bPerformTagBasedFiltering,
		TEXT("If true, the status icon processor will apply or remove the visible tag to visible icons for the current frame."),
		ECVF_Default);

};

UStatusIconProcessor::UStatusIconProcessor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//NOTE: We need to inform users somehow that UMassProcessor::ProcessingPhase should be defined in processor constructor as it's used from CDO on processor initialization.
	ProcessingPhase = EMassProcessingPhase::FrameEnd; //Icon screen position processing needs to run after camera updates to be accurate to the current frame.
}

void UStatusIconProcessor::ConfigureQueries()
{	
	StatusIconQuery.RegisterWithProcessor(*this);
	StatusIconQuery.AddRequirement<FIconStateFragment>(EMassFragmentAccess::ReadWrite);
	StatusIconQuery.AddRequirement<FIconProgressFragment>(EMassFragmentAccess::ReadWrite);
	StatusIconQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	StatusIconQuery.AddRequirement<FStatusHealthFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

inline FIntRect::IntPointType ToIntPoint(const FVector2D& VectorPoint)
{
	return FIntRect::IntPointType(VectorPoint.X, VectorPoint.Y);
}

//This is the same as FSceneView::ProjectWorldToScreen but we want the icon "depth" to use for scaling so our out FVector2D is now a FVector.
FORCEINLINE bool ProjectWorldToScreen(const FVector& WorldPosition, const FIntRect& ViewRect, const FMatrix& ViewProjectionMatrix, FVector& OutScreenPosition)
{
	const FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
	if ( Result.W > 0.0f )
	{
		// the result of this will be x and y coords in -1..1 projection space
		const float RHW = 1.0f / Result.W;
		const FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

		// Move from projection space to normalized 0..1 UI space
		const float NormalizedX = ( PosInScreenSpace.X / 2.f ) + 0.5f;
		const float NormalizedY = 1.f - ( PosInScreenSpace.Y / 2.f ) - 0.5f;

		const FVector2D RayStartViewRectSpace(
			( NormalizedX * (float)ViewRect.Width() ),
			( NormalizedY * (float)ViewRect.Height() )
			);

		const FVector2D OutScreenPosition2D = RayStartViewRectSpace + FVector2D(static_cast<float>(ViewRect.Min.X), static_cast<float>(ViewRect.Min.Y));
		OutScreenPosition = FVector(OutScreenPosition2D.X, OutScreenPosition2D.Y, Result.W);
		return true;
	}

	return false;
}

void UStatusIconProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_StatusIcon_ProcessIcons)
	
	UWorld* World = EntityManager.GetWorld();
	
	if(!World)
	{
		return;
	}
	
	APlayerController* LocalPlayerController = World->GetFirstPlayerController();

	if(!LocalPlayerController)
	{
		return;
	}

	ULocalPlayer* const LocalPlayer = LocalPlayerController->GetLocalPlayer();

	if(!LocalPlayer || !LocalPlayer->ViewportClient)
	{
		return;
	}
	
	FSceneViewProjectionData ProjectionData;
	LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData);

	const bool bPerformPreculling = SW::StatusIconProcessor::bPerformPrecullingInProcessor;
	StatusIconQuery.ForEachEntityChunk(EntityManager, Context, [ProjectionData, bPerformPreculling](FMassExecutionContext& LocalContext)
	{
		const TArrayView<FIconStateFragment> IconStateList = LocalContext.GetMutableFragmentView<FIconStateFragment>();
		const TArrayView<FIconProgressFragment> IconProgressList = LocalContext.GetMutableFragmentView<FIconProgressFragment>();
		const TConstArrayView<FTransformFragment> TransformList = LocalContext.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FStatusHealthFragment> HealthList = LocalContext.GetFragmentView<FStatusHealthFragment>();
		
		const bool bHasHealthEntries = HealthList.Num() != 0;

		const int32 NumEntities = LocalContext.GetNumEntities();

		TSharedPtr<FMassCommandBuffer> Buffer = LocalContext.GetSharedDeferredCommandBuffer();

		const FIntRect& ViewRectangle = ProjectionData.GetConstrainedViewRect();
		FMatrix const ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
		FVector EntityScreenPosition = FVector(-UE_MAX_FLT);

		for(int32 Index = NumEntities - 1; Index >= 0; Index--)
		{
			FIconStateFragment& IconState = IconStateList[Index];
			FIconProgressFragment& IconProgress = IconProgressList[Index];
			const FTransformFragment& TransformData = TransformList[Index];

			const FVector WorldPosition = TransformData.GetTransform().TransformPosition(IconState.WorldOffset);
			
			if(!ProjectWorldToScreen(WorldPosition, ViewRectangle, ViewProjectionMatrix, EntityScreenPosition))
			{
				IconState.ScreenPosition = FVector2D(-UE_MAX_FLT);
				continue;
			}

			IconState.ScreenPosition = { EntityScreenPosition.X, EntityScreenPosition.Y };

			constexpr float ScaleStartDistance = 2000.f; 
			constexpr float ScaleDistanceRate = 2500.f;
			constexpr float MinDistanceScale = 0.5f;

			if(IconState.DrawScale != -1.f)
			{
				const float CurrentScale = FMath::InterpStep(1.f, 0.f, ((EntityScreenPosition.Z - ScaleStartDistance) / ScaleDistanceRate), 8);
				IconState.DrawScale = FMath::Clamp(CurrentScale, MinDistanceScale, 1.f);
			}

			const float IconDrawScale = IconState.DrawScale != -1.f ? IconState.DrawScale : 1.f;
			float ProgressValue = -1.f;

			switch(IconProgress.TrackedValue)
			{
			case EIconProgressValueSelector::Health:
				ProgressValue = bHasHealthEntries ? HealthList[Index].Health / HealthList[Index].MaxHealth : -1.f;
				break;
			default:
				ProgressValue = -1.f;
				break;
			}
			
			if(ProgressValue == -1.f)
			{
				IconState.ScreenPosition = FVector2D(-UE_MAX_FLT);
				continue;
			}
			
			//Lets try to precull status icons here.
			if(bPerformPreculling)
			{
			#if WITH_EDITOR
				//Editor has an issue where the given rectangle is not 100% accurate.
				const FVector2D EditorExtentBoost = GEditor ? FVector2D(16.f, 32.f) : FVector2D(0.f);
				const FVector2D TotalIconHalfSize = (IconState.ExtentHalfSize * IconDrawScale) + EditorExtentBoost;
			#else
				const FVector2D TotalIconHalfSize = IconState.ExtentHalfSize * IconDrawScale;
			#endif
							
				const FIntRect::IntPointType StatusIconTopLeft = FIntRect::IntPointType(ToIntPoint(IconState.ScreenPosition - TotalIconHalfSize));
				const FIntRect::IntPointType StatusIconBottomRight = FIntRect::IntPointType(ToIntPoint(IconState.ScreenPosition + TotalIconHalfSize));
				const FIntRect StatusIconRect = FIntRect(StatusIconTopLeft, StatusIconBottomRight);
				if(!ViewRectangle.Intersect(StatusIconRect))
				{
					IconState.ScreenPosition = FVector2D(-UE_MAX_FLT);
					continue;
				}
			}
			
			IconProgress.Progress = ProgressValue;
		}
	});
}

UHealthRandomizerProcessor::UHealthRandomizerProcessor()
{
	ObservedType = FStatusHealthFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UHealthRandomizerProcessor::ConfigureQueries()
{
	HealthRandomizerQuery.RegisterWithProcessor(*this);
	HealthRandomizerQuery.AddRequirement<FStatusHealthFragment>(EMassFragmentAccess::ReadWrite);
}

void UHealthRandomizerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	HealthRandomizerQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& LocalContext)
	{
		const TArrayView<FStatusHealthFragment> HealthList = LocalContext.GetMutableFragmentView<FStatusHealthFragment>();

		for(FStatusHealthFragment& Health : HealthList)
		{
			Health.Health = FMath::RandRange(0.0, Health.MaxHealth);
		}
	});
}