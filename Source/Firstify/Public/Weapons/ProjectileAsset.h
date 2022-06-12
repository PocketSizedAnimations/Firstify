// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Firstify.h"
#include "Chaos/ChaosEngineInterface.h"
#include "ProjectileAsset.generated.h"



USTRUCT(BlueprintType)
struct FImpactFX
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Impact")
		TArray<class UMaterialInterface*> HitDecals;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Impact")
		TArray<class UMaterialInterface*> HitDecalsRare;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Impact")
		float RareChancePercent;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Impact")
		float DecalSize;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Impact")
		class UNiagaraSystem* ImpactFX;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Impact")
		class USoundCue* ImpactSound;


	FImpactFX()
	{
		//HitDecal = nullptr;
		RareChancePercent = 0.25f;
		DecalSize = 1.0f;
		ImpactFX = nullptr;
		ImpactSound = nullptr;
	}

	UMaterialInterface* ReturnHitDecal()
	{
		UMaterialInterface* ReturnMat = nullptr;

		

		/*chance for rare decal*/
		if (HitDecalsRare.Num() > 0)
		{
		
			float RandRoll = FMath::RandRange(0.0f, 1.0f);

			//regular decal
			if (RandRoll > RareChancePercent)
			{
				
				float RandIndex = FMath::RandRange(0, HitDecals.Num() - 1);
				ReturnMat = HitDecals[RandIndex];
			}
			//rare decal
			else
			{
			
				float RandIndex = FMath::RandRange(0, HitDecalsRare.Num() - 1);
				ReturnMat = HitDecalsRare[RandIndex];
			}
		}
		/*normal decals only*/
		else if(HitDecals.Num() > 0)
		{
			
			float RandIndex = FMath::RandRange(0, HitDecals.Num() - 1);

			ReturnMat = HitDecals[RandIndex];
		}
		

		return ReturnMat;
	}

	
	float ReturnDecalSize()
	{
		return FMath::RandRange(DecalSize * 0.75f, DecalSize);
	}
};

USTRUCT(BlueprintType)
struct FImpactFXList
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PhysicalProperties)
		TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PhysicalProperties")
		FImpactFX FX;

};

UENUM(BlueprintType)
enum class EImpactFXSize : uint8
{
	Small,
	Medium,
	Large
};

/**
 * 
 */
UCLASS(BlueprintType)
class FIRSTIFY_API UProjectileAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projetile / Bullet")
		float Damage;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projetile / Bullet")
		TSubclassOf<class UDamageType> DamageType;
	//how many individual bullets/pellets are fired per shot (for shotgun type blasts)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile / Bullet")
		uint8 BulletCount;
	//How fast the bullet travels (in meters)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile / Bullet")
		float BulletVelocity;
	//Effective range (in meters) - before gravity starts to take ahold and the bullet drops
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile / Bullet")
		float EffectiveRange;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Physics")
		float ImpactMomentum;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "FX|Tracer")
		class UNiagaraSystem* TracerFX;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "FX|Tracer")
		float TracerVelocity;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Hit Effects")
		FImpactFX DefaultFX;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Hit Effects")
		FImpactFX BloodFX;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Hit Effects")
		TArray<FImpactFXList> ImpactFXs;
	//how far back the blood will travel before dropping
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Blood Effects")
		float ExitWoundRange;
	//FX identifier for when we hit a character
	UPROPERTY(EditDefaultsOnly, Category = "Blood Effect")
		EImpactFXSize ImpactFXSize;



	UProjectileAsset();
};
