// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/*collision*/
#include "Runtime/Engine/Public/CollisionQueryParams.h"

/*pawn*/
#include "GameFramework/Pawn.h"

#include "GameFramework/Info.h"
#include "Weapons/ProjectileAsset.h"
#include "GameFramework/Controller.h"



#include "ProjectileManager.generated.h"

USTRUCT(BlueprintType)
struct FProjectile
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		class UProjectileAsset* ProjectileData;
	//Where this Projectile Currently Is
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		FVector Location;
	////Direction it's traveling
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		FVector ForwardDirection;
	//how much gravity is currently affecting us
	UPROPERTY()
		float GravityStrength;
	//How fast it's traveling per second
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		float Velocity;
	//how fast the tracer is traveling
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		float TracerVelocity;
	//How far it's traveled from its origin
	UPROPERTY()
		float DistanceTraveled;
	//How far we can travel before gravity takes hold and begins dropping our trajectory
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		float EffectiveRange;
	UPROPERTY() //Allows us to override the gravity affect (9.8) if need be
		float BulletDropRate;

	/*determines if the projectile has hit anything - and should be ignored until cleanup*/
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
		bool bProjectileHit;
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
		FVector HitLocation;	
	/*how long since we've made contact - used for cleanup*/
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
		float TimeSinceHit;

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		TArray<class AActor*> IgnoredActors;
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		AController* Instigator;
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		AActor* DamageCauser;

	//**tracer information**
	UPROPERTY()
		class UNiagaraComponent* Tracer;

	FProjectile() {};
	FProjectile(UProjectileAsset* inProjectileData, FVector startLoc, FVector startDirection, AActor* dmgCauser, AController* inInstigator) : 
		ProjectileData(inProjectileData), 
		Location(startLoc),
		ForwardDirection(startDirection),
		Instigator(inInstigator),
		DamageCauser(dmgCauser)
		{
			bProjectileHit = false;
			DistanceTraveled = 0;
			BulletDropRate = 980;
			GravityStrength = 0;
			if (ProjectileData)
			{
				EffectiveRange = ProjectileData->EffectiveRange;
				Velocity = ProjectileData->BulletVelocity;
				TracerVelocity = ProjectileData->TracerVelocity;
			}
			
			/*add collision exceptions*/
			IgnoredActors.Add(dmgCauser);
			if (inInstigator)
				IgnoredActors.Add(inInstigator->GetPawn());

			Tracer = nullptr;
	};
};

USTRUCT(BlueprintType)
struct FProjectileTrajectory
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Projectile Trajectory")
		FVector StartLocation;
	UPROPERTY(BlueprintReadWrite, Category = "Projectile Trajectory")
		FVector StartDirection;

	/**/
	FProjectileTrajectory() : 
		StartLocation(FVector(0)),
		StartDirection(FVector(1,0,0))
	{};

	FProjectileTrajectory(FVector startLoc, FVector startDir) :
		StartLocation(startLoc),
		StartDirection(startDir)
	{}
};

USTRUCT()
struct FImpactDecal
{
	GENERATED_BODY()
public:
	
	UPROPERTY()
		class UMaterialInterface* DecalMaterial;
	UPROPERTY()
		float DecalSize;

	FImpactDecal() {
		DecalMaterial = nullptr;
		DecalSize = 1.0f;
	}

	FImpactDecal(class UMaterialInterface* mat, float size)
	{
		DecalMaterial = mat;		
		DecalSize = size;
	}
};


/**
 * 
 */
UCLASS()
class FIRSTIFY_API AProjectileManager : public AInfo
{
	GENERATED_BODY()
protected:

	UPROPERTY()
		TArray<FProjectile> Projectiles;
	//collision defaults
		FCollisionQueryParams BulletCollisionParams;

	UPROPERTY()
		bool bDebugProjectiles;
	UPROPERTY()
		bool bShowBulletTrace;

	UPROPERTY()
		TArray<class UNiagaraComponent*> TracerComponents;
	UPROPERTY()
		class UNiagaraComponent* Test;

	//=========================================================================================
	//========================================FUNCTIONS========================================
	//=========================================================================================
	AProjectileManager();

	//================================
	//==============TICK==============
	//================================
	/*moves all projectiles throughout the frame*/
	virtual void Tick(float DeltaTime) override;

	//===========================================
	//============PROJECTILE CREATION============
	//===========================================
public:
	UFUNCTION()
		void SpawnProjectile(FProjectile newProjectile);

	//===================================================
	//==============PENTRATION && COLLISION==============
	//===================================================
protected:
	UFUNCTION()
		virtual void OnProjectileImpact(FHitResult HitResults, FProjectile Projectile);
	UFUNCTION()
		virtual void InflictDamage(FHitResult HitResults, FProjectile Projectile);

	UFUNCTION()
		virtual class UPhysicalMaterial* ReturnPhysicalMaterial(FHitResult HitResults);
	UFUNCTION()
		virtual void ApplyImpactForce(FHitResult HitResults, FProjectile Projectile);
	
	//==============================
	//==============FX==============
	//==============================
	UFUNCTION()
		virtual void ClearTracer(class UNiagaraComponent* TracerComp);
	UFUNCTION()
		virtual void SpawnImpactFX(class UNiagaraSystem* HitFX, FVector HitLocation, FVector HitNormal);
	UFUNCTION()
		virtual void SpawnImpactDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial);
		virtual void SpawnImpactDecal(UMaterialInterface* ImpactDecalMat, FVector HitLocation, FVector HitNormal,class UPrimitiveComponent* HitComponent = nullptr, float DecalSize = 1.0f);
	UFUNCTION()
		virtual void SpawnBloodSplatterDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial);
	UFUNCTION()
		virtual void SpawnBloodTrailDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial);
	UFUNCTION()
		virtual void SpawnExitWoundDecal(FHitResult HitResults, FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial);	
	UFUNCTION()
		virtual FImpactDecal ReturnImpactMaterial(FProjectile ProjectileInfo, UPhysicalMaterial* PhysicalMaterial);
	UFUNCTION()
		virtual void PlayImpactSound(class USoundBase* ImpactSound, FVector SoundLocation);

	//===================================
	//===========NOTIFICATIONS===========
	//===================================
public:
	UFUNCTION()
		virtual void NotifyPlayerOfHit(AFirstifyPlayerController* PlayerController, FHitResult HitResults);

	//=================================
	//==============DEBUG==============
	//=================================
public:
	UFUNCTION()
		virtual void ToggleDebugTracing();
};
