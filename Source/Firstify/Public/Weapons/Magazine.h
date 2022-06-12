// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"

UCLASS(Blueprintable, abstract)
class FIRSTIFY_API AMagazine : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ammo Setup")
		class UProjectileAsset* AmmoClass;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ammo Setup", meta = (UIMin="0", ClampMin="0"))
		int32 RoundsInMagazine;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ammo Setup", meta = (UIMin = "0", ClampMin = "0"))
		int32 RoundCapacity;
	UPROPERTY()
		class USceneComponent* SceneComponent;
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		class UStaticMeshComponent* FullMagMesh;
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		class UStaticMeshComponent* EmptyMagMesh;
	
public:	
	// Sets default values for this actor's properties
	AMagazine(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
