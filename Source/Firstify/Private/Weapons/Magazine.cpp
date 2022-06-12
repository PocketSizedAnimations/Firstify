// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Weapons/Magazine.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AMagazine::AMagazine(const FObjectInitializer& ObjectInitializer)
{
	SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
	if (SceneComponent)
	{
		SetRootComponent(SceneComponent);
	}

	FullMagMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("FullMagComp"));
	if (FullMagMesh)
	{
		FullMagMesh->SetupAttachment(GetRootComponent());
	}

	EmptyMagMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("EmptyMagMesh"));
	if(EmptyMagMesh)
	{
		EmptyMagMesh->SetupAttachment(GetRootComponent());
	}


	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMagazine::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagazine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

