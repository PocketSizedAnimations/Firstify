// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Blutilities/ObjectivesBlueprintLibrary.h"
#include "Objectives/ObjectiveInfo.h"

/*world*/
#include "Engine/World.h"

AObjectiveInfo* UObjectivesBlueprintLibrary::CreateObjective(UObject* WorldContextObject, TSubclassOf<class AObjectiveInfo> ObjectiveClass, FName Name, FText Description)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	AObjectiveInfo* NewObjective = WorldContextObject->GetWorld()->SpawnActor<AObjectiveInfo>(ObjectiveClass, SpawnParams);

	if (NewObjective)
	{
		NewObjective->Name = Name;
		NewObjective->Description = Description;
	}
	
	return NewObjective;
}

