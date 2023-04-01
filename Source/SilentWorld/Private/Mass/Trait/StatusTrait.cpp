// Copyright Epic Games, Inc. All Rights Reserved.


#include "Mass/Trait/StatusTrait.h"
#include "MassEntityTemplateRegistry.h"

UStatusTrait::UStatusTrait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UStatusTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FStatusHealthFragment& StatusFragment = BuildContext.AddFragment_GetRef<FStatusHealthFragment>();
	StatusFragment.Health = MaxHealth;
	StatusFragment.MaxHealth = MaxHealth;
}