// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ObjectivesBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UObjectivesBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:




	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objectives", meta = (DeterminesOutputType = "ObjectiveClass", WorldContext="WorldContextObject", DefaultToSelf="WorldContextObject"))
		static AObjectiveInfo* CreateObjective(UObject* WorldContextObject, TSubclassOf<class AObjectiveInfo> ObjectiveClass, FName Name, FText Description);
	
};
