// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/Trait/RandomizedPatrolTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Mass/Trait/RandomizedTransformTrait.h"

URandomizedPatrolTrait::URandomizedPatrolTrait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void URandomizedPatrolTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	if(PatrolLength <= 0.f || PatrolSpeed <= 0.f)
	{
		return;
	}
	
	FRandomizedPatrolFragment& RandomizedTransformFragment = BuildContext.AddFragment_GetRef<FRandomizedPatrolFragment>();
	RandomizedTransformFragment.PatrolLength = PatrolLength;
	RandomizedTransformFragment.PatrolSpeed = PatrolSpeed;
	RandomizedTransformFragment.PatrolAccel = PatrolAccel;
}

void URandomizedPatrolTrait::ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{

}

URandomizedPatrolInitializationProcessor::URandomizedPatrolInitializationProcessor()
{
	ObservedType = FRandomizedPatrolFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionOrder.ExecuteAfter.Add(UTransformRandomizerProcessor::StaticClass()->GetFName());
}

void URandomizedPatrolInitializationProcessor::ConfigureQueries()
{
	PatrolRandomizerQuery.RegisterWithProcessor(*this);
	PatrolRandomizerQuery.AddRequirement<FRandomizedPatrolFragment>(EMassFragmentAccess::ReadWrite);
	PatrolRandomizerQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void URandomizedPatrolInitializationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	PatrolRandomizerQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& LocalContext)
	{
		const TArrayView<FRandomizedPatrolFragment> RandomizationList = LocalContext.GetMutableFragmentView<FRandomizedPatrolFragment>();
		const TArrayView<FTransformFragment> TransformList = LocalContext.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = LocalContext.GetNumEntities();
		
		for(int32 Index = 0; Index < NumEntities; Index++)
		{
			FRandomizedPatrolFragment& Randomization = RandomizationList[Index];

			const FRotator ResultingPatrolRotation =
			{
				0.f,
				FMath::RandRange(0.f, 360.f),
				0.f
			};
			
			Randomization.PatrolLength = FMath::RandRange(Randomization.PatrolLength * 0.5f, Randomization.PatrolLength);
			Randomization.PatrolSpeed = FMath::RandRange(Randomization.PatrolSpeed * 0.2f, Randomization.PatrolSpeed);
			Randomization.PatrolStart = TransformList[Index].GetTransform().GetLocation() + (ResultingPatrolRotation.Vector() * (-Randomization.PatrolLength * 0.5f));
			Randomization.PatrolEnd = TransformList[Index].GetTransform().GetLocation() + (ResultingPatrolRotation.Vector() * (Randomization.PatrolLength * 0.5f));

			TransformList[Index].GetMutableTransform().SetTranslation(Randomization.PatrolStart + ((Randomization.PatrolEnd - Randomization.PatrolStart) * FMath::FRand()));
		}
	});
}

URandomizedPatrolProcessor::URandomizedPatrolProcessor()
	: Super()
{
}

void URandomizedPatrolProcessor::ConfigureQueries()
{	
	PatrolQuery.RegisterWithProcessor(*this);
	PatrolQuery.AddRequirement<FRandomizedPatrolFragment>(EMassFragmentAccess::ReadWrite);
	PatrolQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void URandomizedPatrolProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	PatrolQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& LocalContext)
	{
		constexpr float MaxDeltaTime = 0.05f;
		const float DeltaTime = FMath::Min(LocalContext.GetDeltaTimeSeconds(), MaxDeltaTime);
		const TArrayView<FRandomizedPatrolFragment> PositionList = LocalContext.GetMutableFragmentView<FRandomizedPatrolFragment>();
		const TArrayView<FTransformFragment> TransformList = LocalContext.GetMutableFragmentView<FTransformFragment>();

		const int32 NumEntities = LocalContext.GetNumEntities();
		for(int32 Index = NumEntities - 1; Index >= 0; Index--)
		{
			FRandomizedPatrolFragment& PatrolData = PositionList[Index];
			FTransform& Transform = TransformList[Index].GetMutableTransform();
			const FVector TransformPosition = Transform.GetLocation();
			
			const FVector& MoveDestination = PatrolData.Direction == EPatrolDirection::Forward ? PatrolData.PatrolEnd : PatrolData.PatrolStart;
			const FVector MoveDirection = (MoveDestination - TransformPosition).GetSafeNormal();

			const float AccelRate = FVector::DotProduct(MoveDirection, PatrolData.MoveVelocity.GetSafeNormal()) < 0.f ? PatrolData.PatrolAccel * 16.f : PatrolData.PatrolAccel;
			
			PatrolData.MoveVelocity = FMath::VInterpTo(PatrolData.MoveVelocity, PatrolData.MoveVelocity + MoveDirection, DeltaTime, AccelRate);

			Transform.SetTranslation(TransformPosition + (PatrolData.MoveVelocity * DeltaTime));
			
			if((MoveDestination - Transform.GetLocation()).IsNearlyZero(10.f))
			{
				PatrolData.Direction = PatrolData.Direction == EPatrolDirection::Forward ? EPatrolDirection::Backward : EPatrolDirection::Forward;
			}
		}
	});
}