// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Gadgets/DeployableGadget.h"

/*debugging*/
#include "DrawDebugHelpers.h"

/*inventory*/
#include "InventoryManager/InventoryItemComponent.h"

/*players*/
#include "Players/FirstifyCharacter.h"
#include "Players/Components/FirstifyCameraComponent.h"

/*network*/
#include "Net/UnrealNetwork.h"


/*utilities*/
#include "CollisionQueryParams.h"



ADeployableGadget::ADeployableGadget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	CollisionChannel = ECollisionChannel::ECC_GameTraceChannel3;
	UseCooldown = 3.0f;
	MaxDeployDistance = 500.0f;
}

//=====================================================
//=====================INTERACTION=====================
//=====================================================

/*master function - will call all neccessary functions to deploy*/
void ADeployableGadget::Deploy()
{
	/*clients*/
	if (GetNetMode() == NM_Client)
	{
		ServerRequestDeploy();
		return; 
	}
	/**clients end here**/
	
	/*init variables*/
	FHitResult HitResults;

	/*spawn + deploy*/
	if(ADeployableGadget* NewGadget = SpawnGadget(HitResults))
	{
		NewGadget->MulticastOnDeployed(GetOwner(), HitResults);
		DecrementGadgetCount();
	}
}

bool ADeployableGadget::ServerRequestDeploy_Validate()
{
	return true;
}


void ADeployableGadget::ServerRequestDeploy_Implementation()
{
	Deploy();
}

void ADeployableGadget::MulticastOnDeployed_Implementation(AActor* User, FHitResult HitResults)
{	
	
	/*blueprint hook*/
	BP_OnDeployed(User, HitResults);
}

void ADeployableGadget::OnQuickuse(AActor* User)
{
	/*just auto-call Deploy*/
	Deploy();
}

//====================================================
//=====================DEPLOYMENT=====================
//====================================================


ADeployableGadget* ADeployableGadget::SpawnGadget(FHitResult& OutHitResults)
{
	AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetOwner());

	/*safety check*/
	if (!Char)
		return nullptr;

	FVector TraceStart = Char->CameraComponent->GetComponentLocation();
	FVector TraceDirection = Char->CameraComponent->GetComponentRotation().Vector();
	FVector TraceEnd = TraceStart + (TraceDirection * MaxDeployDistance);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());


	if (GetWorld()->LineTraceSingleByChannel(OutHitResults, TraceStart, TraceEnd, CollisionChannel, CollisionParams))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		ADeployableGadget* Gadget = GetWorld()->SpawnActor<ADeployableGadget>(GetClass(), OutHitResults.Location, OutHitResults.ImpactNormal.Rotation(), SpawnParams);

		if (Gadget)
		{
			Gadget->SetHidden(false);
		}

		if (OutHitResults.GetActor())
		{
			Gadget->AttachToActor(OutHitResults.GetActor(), FAttachmentTransformRules::KeepWorldTransform);
		}

		return Gadget;
	}

	/*failed to spawn*/
	return nullptr;
}



