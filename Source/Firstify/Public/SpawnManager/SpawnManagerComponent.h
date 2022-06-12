// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnManagerComponent.generated.h"


UCLASS( ClassGroup=(Spawning), meta=(BlueprintSpawnableComponent) )
class FIRSTIFY_API USpawnManagerComponent : public UActorComponent
{
	GENERATED_BODY()
protected:

	UPROPERTY(Replicated)
		bool bShouldSpawn;
	
	/*random-spawning*/
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100))
		uint8 SpawnChance;

	/*spawn-location modification*/
	UPROPERTY(EditAnywhere, Category = "Spawning|Spawn Offset")
		bool bRandomizeSpawnLocation;
	/*how far we're allowed to spawn from where we're at*/
	UPROPERTY(EditAnywhere, Category = "Spawning|Spawn Offset", meta=(UIMin=0,ClampMin = 0))
		int32 DistanceFromOrigin;

public:	
	// Sets default values for this component's properties
	USpawnManagerComponent();

protected:
	virtual void InitializeComponent() override;
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spawning")
		bool ShouldSpawn();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spawning")
		void RandomizeSpawnLocation();
};
