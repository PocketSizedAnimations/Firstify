// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ShooterWeapon.h"
#include "Weapons/ShooterWeaponInstigatorInterface.h"


/*audio*/
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

/*animations*/
#include "Animation/AnimInstance.h"
#include "Animations/FirstPersonAnimInstance.h"
#include "Animations/Components/FirstPersonArmsComponent.h"

/*inventory*/
#include "InventoryManager/InventoryManagerComponent.h"
#include "Weapons/Magazine.h"

/*debugging*/
#include "Firstify.h"
#include "DrawDebugHelpers.h"

/*fx*/
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"


/*players*/
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Players/FirstifyCharacter.h"
#include "Players/FirstifyPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

/*projectiles*/
#include "Weapons/ProjectileManager.h"
#include "Weapons/ProjectileAsset.h"
#include "Weapons/ProjectileAssaultRifle.h"

/*network*/
#include "Net/UnrealNetwork.h"

/*rendering*/
#include "Components/SkeletalMeshComponent.h"

/*timers*/
#include "TimerManager.h"

/*utilities*/
#include "Kismet/GameplayStatics.h"
#include "Math/Vector.h"


/*world*/
#include "GameInfo/FirstifyGameState.h"

void AShooterWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterWeapon, BulletSpreadSeed,COND_InitialOrOwner);
}


AShooterWeapon::AShooterWeapon(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bTriggerDepressed = false;
	bFiring = false;

	AimDownSightsTime = 0.80f;

	bUsesMagazines = true;
	MagazinesOnSpawn = 6;

	ReloadType = EReloadType::Magazine;
	RoundsPerMagazine = 30;
	RoundsInMagazine = RoundsPerMagazine;
	

	ProjectileAsset = NewObject<UProjectileAssaultRifle>();
	FireMode = EFireMode::Semi;
	FireRate = 600.0f;
	bHasSemiAutoMode = true;
	bHasBurstMode = false;
	bHasFullAutoMode = false;

	FireTimeStamp = 0;
	EndFireTimeStamp = 0;

	MaxBulletSpread = 26;
	MinBulletSpread = 4;
	BulletSpreadRate = 40.0f;
	
	SpreadReductionDelay = 0.15f;
	SpreadReductionRate = 30.0f;

	MovementSpreadAmount = 28.0f;
	MovementSpreadRate = 50.0f;


	/*audio*/
	FireFadeOutDuration = 0.015f;

	/*animations*/
	ForwardMovementScale = 1.0f;


	/*mesh*/
	MeshMagazine = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MagMeshComp"));
	if (Mesh1P && MeshMagazine)
	{
		if(Mesh1P->SkeletalMesh)
			MeshMagazine->SetupAttachment(Mesh1P,FName("MagazineSocket"));
	}
}

void AShooterWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*adjusts the "recoil" (accuracy) one might experienced based on stances/movement/firing*/
	CalcRecoil(DeltaTime);
	/*determine if our muzzle is blocked in world-space*/
	CalcMuzzleCollision();
}


void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	/*initialize defaults*/
	RoundsInMagazine = RoundsPerMagazine;

	/*collision*/
	bMuzzleBlocked = false;
	MuzzleBlockTraceLength = 10.0f;

	/*sockets*/
	MuzzleSocketName = FName("MuzzleFlashSocket");
	
	/*create audio component*/
	FireAudioComponent = NewObject<UAudioComponent>(this);
	TailFireAudioComponent = NewObject<UAudioComponent>(this);
	DryFireAudioComponent = NewObject<UAudioComponent>(this);

	if (FireAudioComponent)
	{
		FireAudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FireAudioComponent->RegisterComponent();
	}

	if ((Mesh1P && IsValid(Mesh1P->SkeletalMesh)) || (Mesh3P && IsValid(Mesh3P->SkeletalMesh)))
	{
		/*create muzzle flash FX component*/
		MuzzleFlashComponent = NewObject<UNiagaraComponent>(this);

		if (MuzzleFlashComponent)
		{
			MuzzleFlashComponent->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, MuzzleSocketName);
			MuzzleFlashComponent->RegisterComponent();
			MuzzleFlashComponent->bAutoActivate = false;
			//MuzzleFlashComponent->OnSystemFinished.AddDynamic(this, &AShooterWeapon::OnMuzzleFlashComplete);

			if (MuzzleFlash)
				MuzzleFlashComponent->SetAsset(MuzzleFlash);
		}
	}

	/*initialize weapon settings*/
	CurrentBulletSpread = MinBulletSpread;

	/*setup fire mode*/
	if (GetNetMode() < NM_Client) //server
	{
		RandomizeFireSeed();
	}
}
//======================================
//================FIRING================
//======================================
void AShooterWeapon::BeginFire()
{	
	if (IsFiring())
		return;
	
	AFirstifyCharacter* OwningChar = Cast<AFirstifyCharacter>(GetOwner());


	/*ensure we have a valid owner to fire*/
	if (!GetOwner())
	{
		EndFire();
		return;
	}

	/*notifies the trigger has been depressed*/
	bTriggerDepressed = true;

	/*server || owning pawn is being controlled by a local player*/
	if (GetNetMode() < NM_Client || (OwningChar && OwningChar->IsLocallyControlled()))
	{
		/*if we're a client - we need to notify the server we're firing*/
		if (GetOwner()->GetNetMode() == NM_Client)
			ServerNotifyBeginFire();

		if (CanFire())
			PerformFire(); //clients still simulate this on their end
		else if (!HasAmmo())
			PlayDryfireSound();
	}
	/*remote players*/
	else
	{
		PerformFire(); //for remote we don't care - we just simulate because we're told to
	}
}

void AShooterWeapon::EndFire()
{
	AFirstifyCharacter* OwningChar = Cast<AFirstifyCharacter>(GetOwner());
	
	/*owning client - notify server we stopped firing*/
	if (GetOwner()->GetNetMode() == NM_Client && (OwningChar && OwningChar->IsLocallyControlled()))
		ServerNotifyEndFire();

	bTriggerDepressed = false;
	bFiring = false; //replicated - will call OnRep_FireChange to all remote clients
	GetWorldTimerManager().ClearTimer(FireRateHandler);
	FireTimeStamp = 0;
	EndFireTimeStamp = GetWorld()->GetTimeSeconds();
	RoundsFired = 0;

	/*server - randomize the next fireseed and replicate it to everyone for syncing*/
	if(GetNetMode() < NM_Client)
		RandomizeFireSeed();


	/*cleanup FX*/
	EndFireSound(); //fades out current shot and plays tail
}


void AShooterWeapon::ServerNotifyBeginFire_Implementation()
{	
	BeginFire();
}

void AShooterWeapon::ServerNotifyEndFire_Implementation()
{	
	EndFire();
}

bool AShooterWeapon::CanFire()
{
	if (!HasAmmo())
		return false;

	if (IsReloading())
		return false;

	if (IsPendingEquip())
		return false;

	/*dead*/
	if (AFirstifyCharacter* Pawn = Cast<AFirstifyCharacter>(GetOwner()))
	{
		if (Pawn->IsDeadOrIncapacitated())
			return false;
	}



	return Super::CanFire();
}

bool AShooterWeapon::IsFiring()
{
	return GetWorldTimerManager().IsTimerActive(FireRateHandler);
}

void AShooterWeapon::PerformFire()
{
	AFirstifyCharacter* OwningChar = Cast<AFirstifyCharacter>(GetOwner());

	/*server && local-clients : keep track of how much ammo we have*/
	if(GetNetMode() < NM_Client || (OwningChar && OwningChar->IsLocallyControlled()))
	{
		/*cancel any firing if we lose our ability to fire*/
		if (!CanFire())
		{
			EndFire();

			if (!HasAmmo())
				PlayDryfireSound();

			return;
		}
	}

	/*queues up another shot*/
	if (bTriggerDepressed && FireMode > EFireMode::Semi)
		InitFireLoopTimer();

	/*marks first shot for calculating recoil*/
	if (FireTimeStamp == 0)
		FireTimeStamp = GetWorld()->GetTimeSeconds();

	/*initiate gunshot*/
	bFiring = true; //@TODO: move this to a cleaner location
	PerformShot();
	ConsumeAmmo();
	RoundsFired++;

	/*FX*/
	if (GetNetMode() != ENetMode::NM_DedicatedServer) //no need for FX to play on Dedicate Servers (they're non-visual)
	{
		PlayMuzzleFlash();
		PlayFireSound();
		PlayRecoilAnimation();
	}

	/*stop firing if we're no longer attempting to fire*/
	if (!GetWorldTimerManager().IsTimerActive(FireRateHandler))
		EndFire();
}

/*InitFireLoopTimer() - queues up another fire as soon as ready*/
void AShooterWeapon::InitFireLoopTimer()
{
	/*stops from overstepping*/
	if (GetWorldTimerManager().IsTimerActive(FireRateHandler))
		return;

	if (FireMode > EFireMode::Semi)
		GetWorldTimerManager().SetTimer(FireRateHandler, this, &AShooterWeapon::PerformFire, ReturnFireRate(), true);
}

/*PerformShot() - creates the actual bullet in the world, and initiates it
*
*
*
*/
void AShooterWeapon::PerformShot()
{
	
	AFirstifyGameState* GS = Cast<AFirstifyGameState>(GetWorld()->GetGameState());

	/*safety check*/
	if (!IsValid(ReturnProjectileManager()))
		return;

	/*safety check*/
	if (!ProjectileAsset)
	{
		UE_LOG(LogFirstify, Warning, TEXT("%s::PerformShot() - !!!NO PROJECTILE DATA FOUND CANNOT FIRE!!!"), *GetName());
		return;
	}

	for(int32 i = 0; i < ProjectileAsset->BulletCount; i++)
	{
		/*get trajectory of the bullet*/
		FProjectileTrajectory Trajectory = ReturnProjectileTrajectory();

		/*spawn the bullet in the world*/
		FProjectile Projectile = FProjectile(ProjectileAsset, Trajectory.StartLocation, Trajectory.StartDirection, this, Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr);//create projectile
		/*ensure we don't accidently shoot ourselves*/
		Projectile.IgnoredActors.Add(GetOwner());
		Projectile.IgnoredActors.Add(this);
		ReturnProjectileManager()->SpawnProjectile(Projectile);
	}
}


//============================================
//===================RECOIL===================
//============================================

void AShooterWeapon::CalcRecoil(float DeltaTime)
{		
	float FireTime = GetWorld()->GetTimeSeconds() - FireTimeStamp;
	float EndFireTime = GetWorld()->GetTimeSeconds() - EndFireTimeStamp;

	APawn* Character = Cast<APawn>(GetOwner());

	if (Character && Character->GetMovementComponent() && Character->GetMovementComponent()->Velocity.Size() > 0)
	{
		CurrentBulletSpread += MovementSpreadRate * DeltaTime;
	}	

	if (bFiring)
	{
		CurrentBulletSpread += BulletSpreadRate * DeltaTime;
	}
	else if(EndFireTime >= SpreadReductionDelay)
	{
		CurrentBulletSpread -= SpreadReductionRate * DeltaTime;
	}

	CurrentBulletSpread = FMath::Clamp(CurrentBulletSpread, MinBulletSpread, MaxBulletSpread);
}

void AShooterWeapon::ApplyRecoil(FVector& FireDirection)
{
	/*first shot accuracy for non-shotgun style weapons*/
	/*if weapon has a "buckshot" style shot (more than one bulletcount) it'll ignore*/
	if((ProjectileAsset->BulletCount == 1) && RoundsFired == 0 && CurrentBulletSpread == MinBulletSpread)
	{
		return; //no accuracy punishment for first shot
	}

	/*apply recoil*/
	float AimError = FMath::DegreesToRadians(CurrentBulletSpread / 10);

	if(AimError > 0)
		FireDirection = BulletSpreadStream.VRandCone(FireDirection, AimError);
}

/*RandomizeFireSeed() - generates a random number and then sends it to all players
* this is used to calculate the bullet spread "randomly" in a replicated way. The Random Seed will create a consistent pattern for all players that can reliably replicate it in their simulations
*
*
*/
void AShooterWeapon::RandomizeFireSeed()
{
	if (HasAuthority())
		MulticastSetFireSeed(FMath::Rand());
}

float AShooterWeapon::ReturnBulletSpreadPercentage()
{
	return (CurrentBulletSpread - MinBulletSpread) / (MaxBulletSpread - MinBulletSpread);
}

void AShooterWeapon::MulticastSetFireSeed_Implementation(int32 NewSeed)
{		
	BulletSpreadSeed = NewSeed;
	BulletSpreadStream.Initialize(NewSeed);
}


//===========================================
//================PROJECTILES================
//===========================================

/*ReturnProjectileManager() - returns the system that always tracks projectiles in our game*/
AProjectileManager* AShooterWeapon::ReturnProjectileManager()
{
	if (AFirstifyPlayerController* PC = Cast<AFirstifyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		return PC->ProjectileManager;
	}
	else
	{
		return nullptr;
	}	
}




/* ReturnProjectileTrajectory() - determines where the gun should start it's trace from
*
*
*
*/
FProjectileTrajectory AShooterWeapon::ReturnProjectileTrajectory()
{
	AFirstifyCharacter* OwningPawn = ReturnOwningPawn();
	AFirstifyPlayerController* PC = ReturnOwningPawn() ? ReturnOwningPawn()->ReturnPlayerController() : nullptr;
	FVector startLocation = GetActorLocation(); //default to our current position
	FVector startDirection = GetActorForwardVector(); //default to our current direction

	///*trace setup*/
	//FCollisionQueryParams CollisionParams = FCollisionQueryParams();
	//CollisionParams.bTraceComplex = true; //Allows more precise bullet mechanics
	//CollisionParams.AddIgnoredActor(this);
	//CollisionParams.AddIgnoredActor(GetOwner());
	//FHitResult HitResults;
	//	
	/*standard firstify pawn*/
	if (ReturnOwningPawn())
	{				
		ReturnOwningPawn()->ReturnFireTrajectory(startLocation, startDirection);
		
	
		//if (Mesh1P->SkeletalMesh)
		//	startLocation = Mesh1P->GetSocketLocation(MuzzleSocketName); //fire from the gun's barrel
		//else
		//	startLocation = GetActorLocation();
		//				
	}

	/*non-player that implements hooks to pull data from*/
	else if (GetOwner() && GetOwner()->Implements<UShooterWeaponInstigatorInterface>())
	{

	}
	/*controller of the owner has the hook instead*/
	else if (GetOwner() && GetOwner()->GetInstigatorController() && GetOwner()->GetInstigatorController()->Implements<UShooterWeaponInstigatorInterface>())
	{
		/*return the the override instead*/
		return IShooterWeaponInstigatorInterface::Execute_ReturnAimingTrajectory(GetOwner()->GetInstigatorController()); 
	}

	///*perform the trace*/
	///*transform where the player is aiming and point the projectile in that direction*/
	//if (GetWorld()->LineTraceSingleByChannel(HitResults, TraceStart, TraceEnd, ECollisionChannel::ECC_GameTraceChannel2, CollisionParams))
	//{
	//	startDirection = HitResults.Location - startLocation;
	//	startDirection.Normalize();
	//	//DrawDebugLine(GetWorld(), TraceStart, HitResults.Location, FColor::Cyan, false, 3.0f);
	//}
	//else
	//{
	//	startDirection = TraceEnd - startLocation;
	//	startDirection.Normalize();
	//}

	/*offset trajectory*/
	ApplyRecoil(startDirection);
	
	return FProjectileTrajectory(startLocation,startDirection);
}

FVector AShooterWeapon::ReturnMuzzleLocation()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	
	if (GetNetMode() < NM_Client || (Pawn && Pawn->IsLocallyControlled()))
		return Mesh1P->GetSocketLocation(MuzzleSocketName);
	else
		return Mesh3P->GetSocketLocation(MuzzleSocketName);
}

void AShooterWeapon::ConsumeAmmo()
{
	RoundsInMagazine--;

	if (CurrentMagazine)
		CurrentMagazine->RoundsInMagazine--;
}

bool AShooterWeapon::HasAmmo()
{
	if (RoundsInMagazine <= 0)
		return false;

	return true;
}

int32 AShooterWeapon::ReturnTotalAmmoForWeapon()
{	
	if (AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetOwner()))
	{
		if (Char->InventoryManager)
			return Char->InventoryManager->ReturnAmmoOnHand(ProjectileAsset);
	}

	/*fallback*/
	return RoundsInMagazine;

}

int32 AShooterWeapon::ReturnMagazinesOnSpawn()
{
	return MagazinesOnSpawn;
}

int32 AShooterWeapon::ReturnAmmoCountOnSpawn()
{
	return RoundsPerMagazine * MagazinesOnSpawn;
}

UProjectileAsset* AShooterWeapon::ReturnAmmoClass()
{
	if (ProjectileAsset)
		return ProjectileAsset;
	else
		return nullptr;
}

//========================================
//================FIRERATE================
//========================================

float AShooterWeapon::ReturnFireRate()
{
	//dividing the result by 1 converts bullets per second (FireRate / 60) to fraction of a second it takes to get 1 bullet out to match the FireRate
	//e.g: 1000 / 60 = 16.66 bullets/sec. So 1 / 16.66 = 0.060s to project 1 bullet out to achieve 16 in a second
	return (1 / (FireRate / 60)); 
}

void AShooterWeapon::AimDownSights()
{

}

void AShooterWeapon::ReleaseSights()
{
}

float AShooterWeapon::ReturnDesiredFOV()
{
	return 60.0f;
}

//=====================================================================
//==============================RELOADING==============================
//=====================================================================

bool AShooterWeapon::CanReload()
{
	/*already fully loaded*/
	if (RoundsInMagazine == RoundsPerMagazine)
		return false;
	
	/*already reloading*/
	if (IsReloading())
		return false;
	
	/*in the middle of weapon switch*/
	if (IsPendingEquip())
		return false;

	
	if (AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetOwner()))
	{
		/*if (Char->IsAimingDownSights())
			return false;*/

		/*if we have no ammo in our inventory (players)*/
		if (Char->GetController()->IsA<APlayerController>() && !Char->InventoryManager->HasAmmoOfType(ProjectileAsset))
			return false;
	}
	return true;
}

void AShooterWeapon::Reload()
{	
	if (!CanReload())
		return;
	
	if (GetNetMode() == NM_Client)
		ServerNotifyReload();

	if (IsFiring())
		EndFire();
		
	/*start reload*/
	BeginReload();
}

void AShooterWeapon::BeginReload()
{
	SetState(EWeaponState::Reloading);
}


bool AShooterWeapon::CanIncrementRound()
{
	/*prevent incremental rounds if not a shell-by-shell reload type*/
	if (ReloadType != EReloadType::Shell)
		return false;

	/*prevent incremental rounds beyond magazine capacity*/
	if ((RoundsInMagazine + 1) > RoundsPerMagazine)
		return false;

	return true;
}

void AShooterWeapon::IncrementRound()
{
	/*check*/
	if (!CanIncrementRound())
		return;

	/*if owning-client - notify server*/
	if (GetNetMode() == NM_Client && ReturnOwningPawn()->IsLocallyControlled())
		ServerNotifyIncrementRound();

	/*increment round*/
	RoundsInMagazine += ReturnOwningPawn()->InventoryManager->ReturnSubtractedAmmo(ProjectileAsset,1);	

	/*check if we should end reloading*/
	if (IsReloading())
	{		
		AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetOwner());

		/*complete reloading if we've reached the max or the reload key was let go*/
		if(RoundsInMagazine == RoundsPerMagazine || Char->bReloadKeyHeld == false)
			EndReload();
	}
		
}

bool AShooterWeapon::ServerNotifyIncrementRound_Validate()
{
	return true;
}

void AShooterWeapon::ServerNotifyIncrementRound_Implementation()
{
	IncrementRound();
}

bool AShooterWeapon::IsReloading()
{
	if (GetWorldTimerManager().IsTimerActive(ReloadHandler) || WeaponState == EWeaponState::Reloading)
		return true;
	else
		return false;
}


float AShooterWeapon::ReturnReloadProgress()
{
	if (!IsReloading())
		return 1.0f;

	float RemainingTime = GetWorldTimerManager().GetTimerRemaining(ReloadHandler);
	float ReloadProgress = (ReloadDuration - RemainingTime) / ReloadDuration; //subtract time remaining from original time, and then divide by total length to get percentage

	return ReloadProgress;
}

bool AShooterWeapon::ServerNotifyReload_Validate()
{
	return true;
}

void AShooterWeapon::ServerNotifyReload_Implementation()
{
	Reload();
}

void AShooterWeapon::OnReloadKeyReleased()
{
	
}

void AShooterWeapon::EndReload()
{
	/*cancel if we're not reloading*/
	if (!IsReloading())
		return;

	/*if owning-client - notify server*/
	if (GetNetMode() == NM_Client && ReturnOwningPawn()->IsLocallyControlled())
		ServerNotifyEndReload();
	
	OnReloadFinished();
}

bool AShooterWeapon::ServerNotifyEndReload_Validate()
{
	return true;
}

void AShooterWeapon::ServerNotifyEndReload_Implementation()
{
	EndReload();
}

void AShooterWeapon::OnReloadFinished()
{
	AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetOwner());

	/*update ammo if server || owning client*/
	if (GetNetMode() < NM_Client || Char->IsLocallyControlled())
	{
		/*magazine reloads*/
		if (ReloadType == EReloadType::Magazine)
		{
			if (Char->InventoryManager && Char->InventoryManager->bTrackAmmoPerMagazine)
			{

				CurrentMagazine = Char->InventoryManager->ReturnNextMagazine(ProjectileAsset);
				RoundsInMagazine = CurrentMagazine->RoundsInMagazine;

			}
			else if (Char->InventoryManager)
			{
				RoundsInMagazine += Char->InventoryManager->ReturnSubtractedAmmo(ProjectileAsset, (RoundsPerMagazine - RoundsInMagazine));
				RoundsInMagazine = FMath::Clamp(RoundsInMagazine, 0, RoundsPerMagazine);
			}
			else /*ai/fallback - just reload what we have*/
			{
				RoundsInMagazine = RoundsPerMagazine;
			}
		}

		SetState(EWeaponState::Equipped);
	}
		
	GetWorldTimerManager().ClearTimer(ReloadHandler);

	/*if we are pending fire after the weapon finishes - let's go ahead and fire*/
	if (Cast<APawn>(GetOwner())->IsLocallyControlled() && bTriggerDepressed)
	{			
		BeginFire();
	}
}

void AShooterWeapon::OnEquippedFinished()
{
	Super::OnEquippedFinished();

	/*fire automatically if we already have the trigger held when we complete our equip animation*/
	if (Cast<APawn>(GetOwner())->IsLocallyControlled() && bTriggerDepressed)
		BeginFire();
}





//=====================================
//================AUDIO================
//=====================================

void AShooterWeapon::MakeNoise(float Loudness, APawn* NoiseInstigator, FVector NoiseLocation, float MaxRange, FName Tag)
{
	//UAiBlueprintLibrary::MakeNoise(GetWorld(), this, ENoiseType::Gunshot, Loudness, NoiseLocation, MaxRange);
}


/**/
void AShooterWeapon::PlayFireSound()
{
	/*safety check*/
	if (!FireAudioComponent || !SemiFireSound)
		return;

	FireAudioComponent->Sound = SemiFireSound;

	/*semi-auto*/
	if(RoundsFired <= 1)
	{ 
		FireAudioComponent->SetBoolParameter("FullAuto", false);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SemiFireSound, GetActorLocation());
		//FireAudioComponent->Play();
	}
	/*full-auto*/
	else if(RoundsFired > 1)
	{
		FireAudioComponent->SetBoolParameter("FullAuto", true); //switch to loop method		
		FireAudioComponent->Play();
	}
	

	/*if (FireTailSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireTailSound, GetActorLocation());
	}*/


	//if (FireSound)
	//	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
	
	/*AI Notification*/
	MakeNoise(1.0f, Cast<APawn>(GetOwner()), GetActorLocation(), 500.0f, FName("NULL"));

	//FireAudioComponent->SetFloatParameter()
}

/*@DEPRECIATED*/
void AShooterWeapon::EndFireSound()
{
	/*fade out any firing shot if it's playing*/
	if (FireAudioComponent->IsPlaying())
		FireAudioComponent->FadeOut(FireFadeOutDuration, 0.0f, EAudioFaderCurve::SCurve);

	FireAudioComponent->SetBoolParameter("FirstShot", true);
}

void AShooterWeapon::PlayDryfireSound()
{
	if (!FireAudioComponent)
		return;

	if (FireAudioComponent->Sound != DryfireSound)
		FireAudioComponent->SetSound(DryfireSound);

	if (!FireAudioComponent->IsPlaying())
		FireAudioComponent->Play();
}

//========================================
//===============ANIMATIONS===============
//========================================

 
void AShooterWeapon::OnAnimNotifyReloadFinished()
{
	/*void out if we're not the owning pawn*/
	if (ReturnOwningPawn() == false || ReturnOwningPawn()->IsLocallyControlled() == false)
		return;

	/*end reload process*/
	EndReload();
}

void AShooterWeapon::OnAnimNotifyInsertShell()
{
	/*void out if we're not the owning pawn*/
	if (ReturnOwningPawn() == false || ReturnOwningPawn()->IsLocallyControlled() == false)
		return;

	/*increment shell by 1*/
	if (CanIncrementRound())
		IncrementRound();
}


void AShooterWeapon::PlayRecoilAnimation()
{	
	UFirstPersonAnimInstance* ArmsInstance = Cast<UFirstPersonAnimInstance>(ReturnArmsAnimInstance());
	ArmsInstance->PlayWeaponFireAnimation();

	

	//UAnimInstance* ArmsAnim = ReturnArmsAnimInstance();

	//if (!ArmsAnim || !ArmsFireAnimation)
	//	return;

	//AFirstifyCharacter* OwningPawn = Cast<AFirstifyCharacter>(GetOwner());
	//UAnimSequenceBase* FireAnim = ArmsFireAnimation;
	//
	//if (OwningPawn && OwningPawn->IsAimingDownSights())
	//	FireAnim = ArmsADSFireAnimation;

	//ArmsAnim->PlaySlotAnimationAsDynamicMontage(FireAnim, FName("DefaultSlot"),0.01f,0.15f);

	//if (Mesh1P && Mesh1P->GetAnimInstance())
	//	Mesh1P->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(WeaponFireAnimation, FName("Slide"), 0.0f, 0.0f);	
}

void AShooterWeapon::PlayReloadAnimation()
{
	UAnimInstance* ArmsInstance = ReturnArmsAnimInstance();
	AFirstifyCharacter* Pawn = Cast<AFirstifyCharacter>(GetOwner());

	if (!ArmsInstance)
		return;
	
	/*first person animations for local player only*/
	if (Pawn && Pawn->IsLocallyControlled())
	{
		if (ArmsReloadEmptyAnimation)
			ArmsInstance->PlaySlotAnimationAsDynamicMontage(ArmsReloadEmptyAnimation, FName("DefaultSlot"), 0.0f, 0.25f);

		if (Mesh1P && Mesh1P->GetAnimInstance())
			Mesh1P->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(WeaponReloadEmptyAnimation, FName("DefaultSlot"), 0.0f, 0.25f);
	}
}

//=====================================================================
//==============================COLLISION==============================
//=====================================================================

/* CalcMuzzleCollision() - Traces from Player Camera to Muzzle Location to determine if we have something in our weapons way
* 
*
*
*/
void AShooterWeapon::CalcMuzzleCollision()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	
	/*trace for weapons that have owning pawns*/
	if (Pawn && Pawn->IsLocallyControlled())
	{
		/*configure initial trace*/
		FHitResult HitResults;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.AddIgnoredActor(GetOwner());

		/*get start spot from camera location*/
		FVector TraceStart; //Camera postion
		FVector MuzzleLoc = ReturnMuzzleLocation(); //MuzzleLocation w/ extra buffer
		FRotator CamRot;
		Pawn->GetController<APlayerController>()->PlayerCameraManager->GetCameraViewPoint(TraceStart, CamRot);
		float DistanceToMuzzle = FVector::Distance(TraceStart, MuzzleLoc); //distance between camera and the muzzle + block trace


		/*perform trace*/
		if (GetWorld()->LineTraceSingleByProfile(HitResults, TraceStart, MuzzleLoc, "Weapon", CollisionParams)) //camera-to-muzzle trace
			bMuzzleBlocked = true;
		else if (GetWorld()->LineTraceSingleByProfile(HitResults, MuzzleLoc, MuzzleLoc + (GetActorForwardVector() * MuzzleBlockTraceLength), "Weapon", CollisionParams)) //muzzle-to-wall trace
			bMuzzleBlocked = true;
		else
			bMuzzleBlocked = false;
	}
}


void AShooterWeapon::PlayMuzzleFlash()
{
	if (!MuzzleFlashComponent || !MuzzleFlash)
		return;

	MuzzleFlashComponent->Activate(true);	
}
