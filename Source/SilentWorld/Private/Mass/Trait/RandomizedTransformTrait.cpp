// Copyright Epic Games, Inc. All Rights Reserved.


#include "Mass/Trait/RandomizedTransformTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

void URandomizedTransformTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FRandomizedTransformFragment& RandomizedTransformFragment = BuildContext.AddFragment_GetRef<FRandomizedTransformFragment>();
	RandomizedTransformFragment.RotationRandomizationMin = bApplyRotation ? RotationRandomizationMin : FRotator(0.f);
	RandomizedTransformFragment.RotationRandomizationMax = bApplyRotation ? RotationRandomizationMax : FRotator(0.f);
	RandomizedTransformFragment.ScaleRandomizationMin = bApplyScale ? ScaleRandomizationMin : FVector(0.f);
	RandomizedTransformFragment.ScaleRandomizationMax = bApplyScale ? ScaleRandomizationMax : FVector(0.f);
	RandomizedTransformFragment.TranslationRandomizationMin = bApplyScale ? TranslationRandomizationMin : FVector(0.f);
	RandomizedTransformFragment.TranslationRandomizationMax = bApplyScale ? TranslationRandomizationMax : FVector(0.f);
}

UTransformRandomizerProcessor::UTransformRandomizerProcessor()
{
	ObservedType = FRandomizedTransformFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UTransformRandomizerProcessor::ConfigureQueries()
{
	TransformRandomizerQuery.RegisterWithProcessor(*this);
	TransformRandomizerQuery.AddRequirement<FRandomizedTransformFragment>(EMassFragmentAccess::ReadOnly);
	TransformRandomizerQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UTransformRandomizerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TransformRandomizerQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& LocalContext)
	{
		const TConstArrayView<FRandomizedTransformFragment> RandomizationList = LocalContext.GetMutableFragmentView<FRandomizedTransformFragment>();
		const TArrayView<FTransformFragment> TransformList = LocalContext.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = LocalContext.GetNumEntities();
		
		for(int32 Index = 0; Index < NumEntities; Index++)
		{
			const FRandomizedTransformFragment& Randomization = RandomizationList[Index];
			FTransformFragment& Transform = TransformList[Index];

			if(Randomization.RotationRandomizationMin != Randomization.RotationRandomizationMax || Randomization.RotationRandomizationMax != FRotator(0.f))
			{
				const FRotator ResultingRotation =
				{
					FMath::RandRange(Randomization.RotationRandomizationMin.Pitch, Randomization.RotationRandomizationMax.Pitch),
					FMath::RandRange(Randomization.RotationRandomizationMin.Yaw, Randomization.RotationRandomizationMax.Yaw),
					FMath::RandRange(Randomization.RotationRandomizationMin.Roll, Randomization.RotationRandomizationMax.Roll)
				};
				Transform.GetMutableTransform().SetRotation(ResultingRotation.Quaternion());
			}
			
			if(Randomization.ScaleRandomizationMin != Randomization.ScaleRandomizationMax || Randomization.ScaleRandomizationMax != FVector(0.f))
			{
				Transform.GetMutableTransform().SetScale3D(Randomization.ScaleRandomizationMin + (FMath::FRand() * (Randomization.ScaleRandomizationMax - Randomization.ScaleRandomizationMin)));
			}

			if(Randomization.TranslationRandomizationMin != Randomization.TranslationRandomizationMax || Randomization.TranslationRandomizationMax != FVector(0.f))
			{
				const FVector ResultingTranslation =
				{
					FMath::RandRange(Randomization.TranslationRandomizationMin.X, Randomization.TranslationRandomizationMax.X),
					FMath::RandRange(Randomization.TranslationRandomizationMin.Y, Randomization.TranslationRandomizationMax.Y),
					FMath::RandRange(Randomization.TranslationRandomizationMin.Z, Randomization.TranslationRandomizationMax.Z)
				};
				Transform.GetMutableTransform().AddToTranslation(ResultingTranslation);
			}
		}
	});
}