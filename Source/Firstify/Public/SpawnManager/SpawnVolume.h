// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SpawnVolume.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTIFY_API ASpawnVolume : public AVolume
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Spawn Volume")
		TArray<TSubclassOf<AActor>> SpawnableClasses;
		


	//=============================================================================
	//==================================FUNCTIONS==================================
	//=============================================================================
public:
		ASpawnVolume();



	
};
