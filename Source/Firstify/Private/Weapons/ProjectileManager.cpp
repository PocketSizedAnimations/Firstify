// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileManager.h"

/*audio*/
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"


/*debugging*/
#include "DrawDebugHelpers.h"
#include "Firstify.h"

/*characters*/
#include "Players/FirstifyCharacter.h"

/*engine*/
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"

/*FX*/
#include "Components/DecalComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraActor.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/classes/NiagaraSystem.h"

/*materials*/
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"

//players
#include "Players/FirstifyPlayerController.h"

//Physics
#include "PhysicalMaterials/PhysicalMaterial.h"

/*world*/
#include "Engine/World.h"



AProjectileManager::AProjectileManager()
{
	bShowBulletTrace = false;
	
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;
}


//================================
//==============TICK==============
//================================

/*Tick() - Moves all projectiles frame by frame
*
*
*
*/
void AProjectileManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugProjectiles)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.015f, FColor::Cyan, FString::Printf(TEXT("ProjectileCount: %i"),Projectiles.Num()));
	}


	/********************************/
	/****LOOP THROUGH PROJECTILES****/
	/********************************/
	for (int i = 0; i < Projectiles.Num(); i++)
	{
		/*initialize*/
		FVector StartLocation = Projectiles[i].Location;
		FVector Direction = Projectiles[i].ForwardDirection;
		float Velocity = Projectiles[i].Velocity * 100; //converts meters to centimeters
		FVector EndLocation = StartLocation + (Direction * Velocity * DeltaTime);
		
		/*projectile has made impact - and is considered dead*/
		if (Projectiles[i].bProjectileHit == true)
		{
			Projectiles[i].TimeSinceHit += DeltaTime;			
		}
		
		/*projectile is still alive - calculate its next position*/
		else
		{			
			FCollisionQueryParams CollisionParams = BulletCollisionParams;
			CollisionParams.bTraceComplex = true; //Allows more precise bullet mechanics
			CollisionParams.bReturnPhysicalMaterial = true;
			CollisionParams.bReturnFaceIndex = true; //allows Physical Materials to be returned per-face (rather than the whole object)
			CollisionParams.AddIgnoredActors(Projectiles[i].IgnoredActors);


			FHitResult HitResult;

			//Gravity Calculations
			if (Projectiles[i].DistanceTraveled > (Projectiles[i].EffectiveRange * 100)) //Only start Gravity after we've surpassed our Effective Range
			{
				if (Projectiles[i].GravityStrength < 920) //A Bullet's Terminal Velocity is rough 91.44m/s
					Projectiles[i].GravityStrength += (Projectiles[i].BulletDropRate * DeltaTime); //Apply 9.8m/s (Gravity)

				EndLocation += (FVector(0, 0, -1) * Projectiles[i].GravityStrength) * DeltaTime; //Apply Downward pull to the bullet's Trajectory			
			}


			/*run hit-trace*/

			//**HIT SURFACE**
			if (Projectiles[i].bProjectileHit == false && GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel2, CollisionParams)) //Check to see if we hit anything
			{
				Projectiles[i].bProjectileHit = true; //mark as pending destroy
				Projectiles[i].HitLocation = HitResult.Location;

				OnProjectileImpact(HitResult, Projectiles[i]);

				//Projectiles.RemoveAt(i);

				if (bShowBulletTrace)
				{
					DrawDebugLine(GetWorld(), StartLocation, HitResult.Location, FColor::Red, false, 1.7f);
					DrawDebugSphere(GetWorld(), HitResult.Location, 4, 8, FColor::Red, false, 1.7f);
				}

			}
			/**NO IMPACT**/
			else
			{				
				Projectiles[i].DistanceTraveled += (Projectiles[i].Location - EndLocation).Size();
				Projectiles[i].Location = EndLocation;				
			}
		}
		


		//**update tracer information**
		if (Projectiles[i].Tracer)
		{
			float TracerVelocity = Projectiles[i].TracerVelocity * 100;
			FVector TracerStartLocation = Projectiles[i].Tracer->GetComponentLocation();
			FVector TracerEndLocation = Projectiles[i].Tracer->GetComponentLocation() + (Direction * TracerVelocity * DeltaTime);
			FVector TracerOrientation = (TracerEndLocation - TracerStartLocation);
			TracerOrientation.Normalize();
			Projectiles[i].Tracer->SetWorldLocation(TracerEndLocation);
			Projectiles[i].Tracer->SetWorldRotation(TracerOrientation.Rotation());

			/*clear tracer if we're close enough to the hit point*/
			if (FVector::Distance(Projectiles[i].Tracer->GetComponentLocation(), Projectiles[i].HitLocation) <= 100 || Projectiles[i].TimeSinceHit > 5.0f)
			{
				ClearTracer(Projectiles[i].Tracer);
				Projectiles.RemoveAt(i);
				continue; //move on to the next loop
			}
		}

		if (bShowBulletTrace)
		{
			//Draw Tracing Visual
			if (Projectiles[i].bProjectileHit == false) 
			{
				FColor DebugColor = FColor::MakeRedToGreenColorFromScalar(1 - (Projectiles[i].GravityStrength / Projectiles[i].BulletDropRate)); //Determines the Bullet's Color based on the amount of Bullet Drop is currently being applied
				DrawDebugLine(GetWorld(), StartLocation, EndLocation, DebugColor, false, 1.75f);
			}

		}	
	}
}



void AProjectileManager::SpawnProjectile(FProjectile newProjectile)
{
	if (bDebugProjectiles)
		UE_LOG(LogFirstify, Log, TEXT("AProjectileManager::SpawnProject()"));

	int32 i = Projectiles.Add(newProjectile);

	if (Projectiles[i].ProjectileData && Projectiles[i].ProjectileData->TracerFX) 
	{
		Projectiles[i].Tracer = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Projectiles[i].ProjectileData->TracerFX, Projectiles[i].Location, Projectiles[i].ForwardDirection.ToOrientationRotator(), FVector(1.0f), false, true);
	}	
}


//===================================================
//==============PENTRATION && COLLISION==============
//===================================================


void AProjectileManager::OnProjectileImpact(FHitResult HitResults, FProjectile Projectile)
{
	/*get the PhysMaterial we hit*/
	UPhysicalMaterial* PhysMaterial = ReturnPhysicalMaterial(HitResults);
	TEnumAsByte<EPhysicalSurface> SurfaceType = PhysMaterial ? PhysMaterial->SurfaceType : EPhysicalSurface::SurfaceType_Default;
	UMaterialInterface* ImpactDecalMat = nullptr;
	float DecalSize = 1.0f;
	UNiagaraSystem* HitFX = nullptr;	
	USoundCue* ImpactSound = nullptr;

	if (bDebugProjectiles)
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, GetNameSafe(HitResults.GetActor()));
	
	
	InflictDamage(HitResults, Projectile);
	//ClearTracer(Projectile.Tracer);


	/*log*/
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPhysicalSurface"), true);
	//if (!EnumPtr) return NSLOCTEXT("Invalid", "Invalid", "Invalid");

	FText SurfaceTypeString = EnumPtr ? EnumPtr->GetDisplayNameText(SurfaceType) : FText::FromString("NULL");
	
	if(bDebugProjectiles)
		UE_LOG(LogFirstify, Log, TEXT("\t SurfaceType : %s"), *SurfaceTypeString.ToString());

	/*fx*/
	/*loop through impacts and find the appropriate one*/
	if (Projectile.ProjectileData->ImpactFXs.Num() > 0)
	{
		int32 i = 0;
		for (auto ImpactFX : Projectile.ProjectileData->ImpactFXs)
		{
			i++;
			
			if(bDebugProjectiles)
				UE_LOG(LogFirstify, Log, TEXT("\t index %i"), i);
			
			
			if (ImpactFX.SurfaceType == SurfaceType)
			{
				SurfaceTypeString = EnumPtr ? EnumPtr->GetDisplayNameText(ImpactFX.SurfaceType) : FText::FromString("NULL");
				
				if(bDebugProjectiles)
					UE_LOG(LogFirstify, Log, TEXT("\t SurfaceType Match Found: SurfaceType(%s)"), *SurfaceTypeString.ToString());

				//ImpactMaterial = ImpactFX.ImpactFX.HitDecals[FMath::RandRange(0, ImpactFX.ImpactFX.HitDecals.Num())];
				//ImpactSize = ProjectileInfo.
				
				HitFX = ImpactFX.FX.ImpactFX;
				ImpactDecalMat = ImpactFX.FX.ReturnHitDecal();
				DecalSize = ImpactFX.FX.ReturnDecalSize();
				ImpactSound = ImpactFX.FX.ImpactSound; //set sound

				/*kill loop*/
				break;
			}
		}
	}
	
	/*fx chain kickoff*/
	SpawnImpactFX(HitFX, HitResults.Location, HitResults.ImpactNormal.GetSafeNormal());
	
	
	SpawnImpactDecal(ImpactDecalMat, HitResults.Location, HitResults.ImpactNormal, HitResults.GetComponent(), DecalSize);
	
	PlayImpactSound(ImpactSound, HitResults.Location);
	
	//old implementations - should be safe to remove shortly
	//SpawnImpactFX(HitResults, Projectile,PhysMaterial);
	//SpawnImpactDecal(HitResults, Projectile, PhysMaterial);
	//PlayImpactSound(HitResults, Projectile, PhysMaterial);

	/*physics*/
	ApplyImpactForce(HitResults, Projectile);

	/*notifications*/
	NotifyPlayerOfHit(Cast<AFirstifyPlayerController>(Projectile.Instigator), HitResults);
}

void AProjectileManager::InflictDamage(FHitResult HitResults, FProjectile Projectile)
{
	AActor* HitActor = HitResults.GetActor();
	if (HitActor)
	{
		HitActor->TakeDamage(Projectile.ProjectileData->Damage, FPointDamageEvent(Projectile.ProjectileData->Damage,HitResults,Projectile.ForwardDirection,Projectile.ProjectileData->DamageType), Projectile.Instigator, Projectile.DamageCauser);
	}
}

UPhysicalMaterial* AProjectileManager::ReturnPhysicalMaterial(FHitResult HitResults)
{

	UPhysicalMaterial* PhysicalMaterial = nullptr;


	//**determine physical material**
	int32 SectionIndex;
	//UE_LOG(LogT1Projectile, Log, TEXT("FaceIndex(%i)"), HitResults.FaceIndex);

	if (HitResults.FaceIndex > -1 && HitResults.GetComponent() && HitResults.GetComponent()->GetMaterialFromCollisionFaceIndex(HitResults.FaceIndex, SectionIndex))
	{
		PhysicalMaterial = HitResults.GetComponent()->GetMaterialFromCollisionFaceIndex(HitResults.FaceIndex, SectionIndex)->GetPhysicalMaterial();
		//UE_LOG(LogT1Projectile, Log, TEXT("Physical Material(%s)"), *GetNameSafe(PhysicsMat));
	}

	if (PhysicalMaterial == nullptr && HitResults.GetComponent() && HitResults.GetComponent()->GetMaterial(0))
	{
		PhysicalMaterial = HitResults.GetComponent()->GetMaterial(0)->GetPhysicalMaterial();
	}

	return PhysicalMaterial;
}

void AProjectileManager::ApplyImpactForce(FHitResult HitResults, FProjectile Projectile)
{
	/*we only want to apply force to actor types*/
	if (HitResults.GetActor() && HitResults.GetComponent() && HitResults.GetComponent()->IsSimulatingPhysics())
	{
		HitResults.GetComponent()->AddImpulseAtLocation(Projectile.ForwardDirection * Projectile.ProjectileData->ImpactMomentum, HitResults.Location,HitResults.BoneName);
	}
}

//==============================
//==============FX==============
//==============================

void AProjectileManager::ClearTracer(UNiagaraComponent* TracerComp)
{
	if (!IsValid(TracerComp))
		return;

	TracerComp->DestroyInstance();
}

void AProjectileManager::SpawnImpactFX(UNiagaraSystem* HitFX, FVector HitLocation, FVector HitNormal)
{	
	/*safety check*/
	if (!HitFX)
		return;

	/*spawn effect*/
	if (IsValid(HitFX))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitFX, HitLocation, HitNormal.Rotation());
}

/* SpawnImpactDecal() - 
*
*
*
*/
void AProjectileManager::SpawnImpactDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial)
{	
	if (bShowBulletTrace)
		UE_LOG(LogFirstify, Log, TEXT("AProjectileManager::SpawnImpactDecal()"));

	FImpactDecal ImpactInfo = ReturnImpactMaterial(ProjectileInfo, PhysicalMaterial);


	/*size-correction*/
	if (ImpactInfo.DecalSize <= 0.0f)
		ImpactInfo.DecalSize = 1.0f;
	
	
	/*player hit - calculate blood splatter location*/
	if (HitResults.GetActor() && HitResults.GetActor()->IsA(APawn::StaticClass()))
	{
		SpawnBloodSplatterDecal(HitResults, ProjectileInfo, PhysicalMaterial);
		SpawnBloodTrailDecal(HitResults, ProjectileInfo, PhysicalMaterial); //blood droplets that would of hit from gravity right behind
		SpawnExitWoundDecal(HitResults, ProjectileInfo, PhysicalMaterial); //majory of blood
	}

	else if(IsValid(ImpactInfo.DecalMaterial))
	{
		FVector HitLocation = HitResults.Location;
	

		UDecalComponent* decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactInfo.DecalMaterial, FVector(3, 8, 8), HitLocation, (HitResults.ImpactNormal * -1).Rotation());

		if (bShowBulletTrace)
		{
			DrawDebugSphere(GetWorld(), HitLocation, 12, 8, FColor::Emerald, false, 4.5f);
			DrawDebugLine(GetWorld(), decal->GetComponentLocation(), decal->GetComponentLocation() + (decal->GetForwardVector() * 8), FColor::Red, false, 4.5f);
			UE_LOG(LogFirstify, Log, TEXT("\t HitDecal : %s"), *GetNameSafe(decal));
		}
		
		if (decal)
		{
			decal->SetFadeScreenSize(0.001f);

			/*apply random rotation*/
			float randomRoll = FMath::RandRange(0.0f, 180.0f);
			decal->AddRelativeRotation(FRotator(0, 0, randomRoll));

			/*apply a random scale to the bullet so it's not exactly the same as all others*/
			float randomScale = FMath::RandRange(ImpactInfo.DecalSize * 0.85f, ImpactInfo.DecalSize * 1.0f);
			decal->SetRelativeScale3D(FVector(randomScale));
			
			if (HitResults.GetComponent())
				decal->AttachToComponent(HitResults.GetComponent(), FAttachmentTransformRules::KeepWorldTransform, NAME_None);
		}		
	}
}

void AProjectileManager::SpawnImpactDecal(UMaterialInterface* ImpactDecalMat, FVector HitLocation, FVector HitNormal,UPrimitiveComponent* HitComponent, float DecalSize)
{
	if (bDebugProjectiles && !ImpactDecalMat)
	{
		UE_LOG(LogFirstify,Warning,TEXT("!!!SpawnImpactDecal() - NO DECAL MATERIAL FOUND!!!"))
		return;
	}

	UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactDecalMat, FVector(3, DecalSize, DecalSize), HitLocation, (HitNormal * -1).Rotation());

	/*randomize decal*/
	if(Decal)
	{ 
		/*makes decals visible from further away*/
		Decal->SetFadeScreenSize(0.001f); //@note : adjusting this makes it either SUPER FAR rendering or SUPER SHORT - need to figure out what a better value is

		/*apply random rotation*/
		float randomRoll = FMath::RandRange(0.0f, 180.0f);
		Decal->AddRelativeRotation(FRotator(0, 0, randomRoll));

		if (HitComponent)
			Decal->AttachToComponent(HitComponent, FAttachmentTransformRules::KeepWorldTransform, NAME_None);
	}
}

void AProjectileManager::SpawnBloodSplatterDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial)
{
	AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(HitResults.GetActor());
	UMaterialInstance* SplatterDecal = nullptr;

	/*UE_LOG(LogFirstify, Log, TEXT("SpawnBloodSplatterDecal()"));*/

	/*get blood decal*/
	//if (HitResults.GetActor() && Char)
	//{
	//	SplatterDecal = Char->ReturnBloodSplatterDecal(ProjectileInfo.ProjectileData->ImpactFXSize);
	//}

	//UE_LOG(LogFirstify, Log, TEXT("Decal(%s)"),*GetNameSafe(SplatterDecal));

	/*spawn decal - front spray*/
	if (SplatterDecal)
	{		
		FVector TraceDirection = (HitResults.TraceEnd - HitResults.TraceStart);
		TraceDirection.Normalize();

		const FVector TraceStart = HitResults.ImpactPoint + (TraceDirection * -80);
		const FVector TraceEnd = TraceStart + (FVector(0, 0, -500));


		FHitResult SplatterTrace;

		if(GetWorld()->LineTraceSingleByChannel(SplatterTrace, TraceStart, TraceEnd, ECollisionChannel::ECC_GameTraceChannel2))
		{ 
			UDecalComponent* BloodSplatter = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), SplatterDecal, FVector(50, 60, 30), SplatterTrace.ImpactPoint);
			
			/*adjust rotation*/
			FVector SplatterDirA = HitResults.TraceStart;
			SplatterDirA.Z = 0;
			FVector SplatterDirB = HitResults.TraceEnd;
			SplatterDirB.Z = 0;

			/*get direction*/
			FVector SplatterDir = (SplatterDirB - SplatterDirA);
			SplatterDir.Normalize();
	

			/*for anyone who sees this - I know no math*/
			BloodSplatter->SetWorldRotation(SplatterDir.Rotation());
			BloodSplatter->AddLocalRotation(FRotator(0, 0, -90));
			BloodSplatter->AddLocalRotation(FRotator(0, -90, 0));
			BloodSplatter->SetFadeScreenSize(0.0001f);
			


			if (bShowBulletTrace) {


				DrawDebugLine(GetWorld(), BloodSplatter->GetComponentLocation(), BloodSplatter->GetComponentLocation() + (BloodSplatter->GetForwardVector() * 150), FColor::Red, false, 2.5f);
				DrawDebugLine(GetWorld(), BloodSplatter->GetComponentLocation(), BloodSplatter->GetComponentLocation() + (BloodSplatter->GetUpVector() * 150), FColor::Blue, false, 2.5f);
				DrawDebugLine(GetWorld(), BloodSplatter->GetComponentLocation(), BloodSplatter->GetComponentLocation() + (BloodSplatter->GetRightVector() * 150), FColor::Green, false, 2.5f);
			}
		}
	}

}

void AProjectileManager::SpawnBloodTrailDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial)
{

}

void AProjectileManager::SpawnExitWoundDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial)
{
	//UE_LOG(LogFirstify, Log, TEXT("ExitWound_1"));
	FVector direction = (HitResults.TraceEnd - HitResults.TraceStart);
	direction.Normalize();


	const FVector traceStart = HitResults.ImpactPoint + (direction * 100);
	const FVector traceEnd = HitResults.ImpactPoint + (direction * ProjectileInfo.ProjectileData->ExitWoundRange);

	if (bShowBulletTrace) 
	{		
		DrawDebugSphere(GetWorld(), traceStart, 8, 8, FColor::Red, false, 5.0f);
		DrawDebugSphere(GetWorld(), traceEnd, 8, 8, FColor::Red, false, 5.0f);
		DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Orange, false, 5.0f);
	}

	/*new decal info*/
	FHitResult bloodHit;
	if (GetWorld()->LineTraceSingleByChannel(bloodHit, traceStart, traceEnd, ECollisionChannel::ECC_GameTraceChannel2))
	{
		//UE_LOG(LogFirstify, Log, TEXT("ExitWound_2"));
		if (bShowBulletTrace)
		{
			DrawDebugSphere(GetWorld(), bloodHit.ImpactPoint, 8, 8, FColor::Emerald, false, 3.0f);
			DrawDebugLine(GetWorld(), HitResults.ImpactPoint, bloodHit.ImpactPoint, FColor::Emerald, false, 3.0f);
		}
		
		
		//uint8 NumberOfDecals = ProjectileInfo.ProjectileData->BloodFX.HitDecal.Num();
		uint8 NumberOfDecals = 1;
		
		/*safety check*/
		if (NumberOfDecals <= 0)
			return;

		/*grab random blood mat*/
		UMaterialInterface* bloodMat = ProjectileInfo.ProjectileData->BloodFX.HitDecals[0];

		

		if (bloodMat)
		{
			//UE_LOG(LogFirstify, Log, TEXT("ExitWound_3"));
			UDecalComponent* decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), bloodMat, FVector(1, 8, 8), bloodHit.ImpactPoint,(bloodHit.ImpactNormal.Rotation() * -1));

			if (decal)
			{
				//UE_LOG(LogFirstify, Log, TEXT("ExitWound_4"));
				/*apply random rotation*/
				float randomRoll = FMath::RandRange(0.0f, 180.0f);
				decal->AddRelativeRotation(FRotator(0, 0, randomRoll));
				/*apply a random scale to the bullet so it's not exactly the same as all others*/
				float randomScale = FMath::RandRange(ProjectileInfo.ProjectileData->BloodFX.DecalSize * 0.85f, ProjectileInfo.ProjectileData->BloodFX.DecalSize * 1.0f);
				decal->SetRelativeScale3D(FVector(randomScale));
			}
		}
	}
}


/*@TODO: REMOVEEEEE MEEEEEEE*/
FImpactDecal AProjectileManager::ReturnImpactMaterial(FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial)
{
	if (!PhysicalMaterial)
		return FImpactDecal();

	FName MaterialType = *GetNameSafe(PhysicalMaterial);
	UMaterialInterface* ImpactMaterial = nullptr;
	float ImpactSize = 1.0f;

	/*loop through impacts and find the appropriate one*/
	if (ProjectileInfo.ProjectileData->ImpactFXs.Num() > 0)
	{
		for (auto ImpactFX : ProjectileInfo.ProjectileData->ImpactFXs)
		{
			if (ImpactFX.SurfaceType == PhysicalMaterial->SurfaceType)
			{
				//ImpactMaterial = ImpactFX.ImpactFX.HitDecals[FMath::RandRange(0, ImpactFX.ImpactFX.HitDecals.Num())];
				//ImpactSize = ProjectileInfo.
			}
		}
	}
	
	/*fallback*/
	if(ImpactMaterial == nullptr)
	{
		ImpactMaterial = ProjectileInfo.ProjectileData->DefaultFX.HitDecals[0];
		ImpactSize = ProjectileInfo.ProjectileData->DefaultFX.DecalSize;
	}

	return FImpactDecal(ImpactMaterial, ImpactSize);
}

void AProjectileManager::PlayImpactSound(USoundBase* ImpactSound, FVector SoundLocation)
{
	/*FName MaterialType = *GetNameSafe(PhysicalMaterial);
	USoundCue* ImpactSound;
	
	
	if (MaterialType == "Concrete")
	{
		ImpactSound = ProjectileInfo.ProjectileData->DefaultFX.ImpactSound;
	}
	else
		ImpactSound = ProjectileInfo.ProjectileData->DefaultFX.ImpactSound;*/

	UGameplayStatics::PlaySoundAtLocation(GetWorld(),ImpactSound,SoundLocation);
}

void AProjectileManager::NotifyPlayerOfHit(AFirstifyPlayerController* PlayerController, FHitResult HitResults)
{
	if (!PlayerController)
		return;

	PlayerController->OnNotifyOfTargetHit(HitResults.GetActor(), HitResults);
}

void AProjectileManager::ToggleDebugTracing()
{
	bShowBulletTrace = !bShowBulletTrace;
	bDebugProjectiles = !bDebugProjectiles;

	if (!bShowBulletTrace)
		FlushPersistentDebugLines(GetWorld());
}
