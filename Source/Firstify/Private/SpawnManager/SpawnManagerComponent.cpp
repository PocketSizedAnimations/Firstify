// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "SpawnManager/SpawnManagerComponent.h"
#include "Firstify.h"

/*bluetilities*/
#include "Blutilities/RaffleBlueprintLibrary.h"


/*network*/
#include "Net/UnrealNetwork.h"

/*world*/
#include "Engine/World.h"


void USpawnManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME_CONDITION(USpawnManagerComponent, bShouldSpawn, COND_InitialOnly);
}

// Sets default values for this component's properties
USpawnManagerComponent::USpawnManagerComponent()
{	
	bRandomizeSpawnLocation = true;
	DistanceFromOrigin = 500;

	SpawnChance = 100;
	
	bShouldSpawn = false;
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void USpawnManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	UE_LOG(LogFirstifyAI, Log, TEXT("%s::InitializeComponent()"), *GetName());
	
	/*set the bool so that clients receive the decision*/
	if (GetNetMode() < NM_Client)
		bShouldSpawn = ShouldSpawn();

	/*destroy the owning actor*/
	if (bShouldSpawn == false)
	{
		UE_LOG(LogFirstifyAI, Log, TEXT("\t Desrtoying Actor..."));
		GetOwner()->Destroy();
		return; //stop further logic
	}

	/*randomize-location*/
	if (bRandomizeSpawnLocation)
		RandomizeSpawnLocation();
}

// Called when the game starts
void USpawnManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USpawnManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


bool USpawnManagerComponent::ShouldSpawn_Implementation()
{
	if (URaffleBlueprintLibrary::RaffleByPercent(SpawnChance))
		return true;

	/*we assume no*/
	return false;
}

void USpawnManagerComponent::RandomizeSpawnLocation_Implementation()
{
	float YOffset, XOffset;
	FVector Origin = GetOwner()->GetActorLocation();
	FVector NewLocation;

	for (int32 i = 0; i < 1500; i++)
	{
		/*random location*/
		XOffset = FMath::RandRange(DistanceFromOrigin * -1, DistanceFromOrigin);
		YOffset = FMath::RandRange(DistanceFromOrigin * -1, DistanceFromOrigin);
		NewLocation = FVector(Origin.X + XOffset, Origin.Y + YOffset, Origin.Z);

		/*check if location is valid*/
		FHitResult HitResults;

		/*automatically use the first spot we have*/
		if (!GetWorld()->SweepSingleByProfile(HitResults, NewLocation, NewLocation, FVector(0, 0, 1).ToOrientationQuat(), "Pawn", FCollisionShape::MakeCapsule(48, 30)))
			break;
	}
		
	/*offset spawn*/
	GetOwner()->SetActorLocation(NewLocation);
}
