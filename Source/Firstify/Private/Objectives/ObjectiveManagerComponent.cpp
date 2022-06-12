// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Objectives/ObjectiveManagerComponent.h"
#include "Firstify.h"


/*network*/
#include "Net/UnrealNetwork.h"


void UObjectiveManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UObjectiveManagerComponent, Objectives, COND_OwnerOnly);
}

// Sets default values for this component's properties
UObjectiveManagerComponent::UObjectiveManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UObjectiveManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UObjectiveManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

AObjectiveInfo* UObjectiveManagerComponent::AddObjective(AObjectiveInfo* Objective)
{
	if (!Objective)
	{
		UE_LOG(LogFirstifyInventory, Warning, TEXT("!!!%s::AddObjective() - FAILED - NULL OBJECTIVE PASSED!!!"), *GetName());
		return nullptr;
	}

	return Objectives[Objectives.Add(Objective)];
}

AObjectiveInfo* UObjectiveManagerComponent::AddObjectiveByInfo(TSubclassOf<class AObjectiveInfo> Objective)
{
	return nullptr;
}

void UObjectiveManagerComponent::CompleteObjective(AObjectiveInfo* Objective)
{
	/*safety check*/
	if (!Objective)
	{
		UE_LOG(LogFirstifyObjectives, Log, TEXT("!!!%s::CompleteObjective() - FAIlED - INVALID OBJECTIVE REFERENCE!!!"));
		UE_LOG(LogFirstifyObjectives, Warning, TEXT("!!!%s::CompleteObjective() - FAIlED - INVALID OBJECTIVE REFERENCE!!!"));
		return;
	}


	for (int32 i = 0; i < Objectives.Num(); i++)
	{
		if (Objectives[i]->Name == Objective->Name)
		{
			Objectives[i]->Status = EObjectiveStatus::Completed;
		}
	}
}

void UObjectiveManagerComponent::CompleteObjectiveByName(FName ObjectiveName)
{

}

void UObjectiveManagerComponent::FailObjective(AObjectiveInfo* Objective)
{
	/*safety check*/
	if (!Objective)
	{
		UE_LOG(LogFirstifyObjectives, Warning, TEXT("!!!%s::FailObjective() - FAIlED - INVALID OBJECTIVE REFERENCE!!!"));
		return;
	}


	for (int32 i = 0; i < Objectives.Num(); i++)
	{
		if (Objectives[i]->Name == Objective->Name)
		{
			Objectives[i]->Status = EObjectiveStatus::Completed;
		}
	}
}

AObjectiveInfo* UObjectiveManagerComponent::ReturnObjective(FName ObjectiveName)
{
	for (AObjectiveInfo* Objective : Objectives)
	{
		if (Objective->Name == ObjectiveName)
			return Objective;
	}
	
	return nullptr;
}



