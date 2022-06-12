// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ProjectileManager.h"
#include "Weapons/Weapon.h"
#include "ShooterWeapon.generated.h"

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single,
	Semi,
	Burst,
	FullAuto
};

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	Magazine,
	Shell
};

/**
 * 
 */
UCLASS(abstract)
class FIRSTIFY_API AShooterWeapon : public AWeapon
{
	GENERATED_BODY()
public:


protected:
	/*firing*/
	UPROPERTY()
		bool bTriggerDepressed;


	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ammunition")
		EReloadType ReloadType;
	/*@DEPRECIATED: to be removed in favor of pure magazine system*/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ammunition|Magazines", meta = (UIMin = "0", ClampMin = "0", EditCondition = bUsesMagazines))
		int32 RoundsPerMagazine;
	UPROPERTY(BlueprintReadOnly, Category = "Ammunition|Magazines", meta=(UIMin = "0", ClampMin = "0", EditCondition = bUsesMagazines))
		int32 RoundsInMagazine;


	/*magazine*/
	UPROPERTY(EditDefaultsOnly, Category = "Ammunition|Magazines")
		bool bUsesMagazines;
	/*the current/default magazine*/
	UPROPERTY()
		class AMagazine* CurrentMagazine;
	/*acceptable magazine types for customization options*/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ammunition|Magazines", meta = (EditCondition = bUsesMagazines))
		TArray<TSubclassOf<class AMagazine>> MagazineTypes;
	/*how many magazines this weapon should spawn with*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ammunition|Magazines", meta = (EditCondition = bUsesMagazines))
		uint8 MagazinesOnSpawn;
	UPROPERTY()
		class UStaticMeshComponent* MeshMagazine;

	/*reloading*/
	FTimerHandle ReloadHandler;
	/*stored value of how long this current animation is going to be*/
	float ReloadDuration;
		
	



	/*rounds*/
	//Rounds Per Minute
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
		float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
		EFireMode FireMode;
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
		bool bHasSemiAutoMode;
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
		bool bHasBurstMode;
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
		bool bHasFullAutoMode;

	//tracks time between shots
	FTimerHandle FireRateHandler; 
	//how many rounds have been fired since we started holding down the trigger
	UPROPERTY(BlueprintReadOnly, Category = "Ammunition")
		int32 RoundsFired;

	/*projectile setup*/
	UPROPERTY(EditDefaultsOnly, Category = "Ammunition|Projectile Setup")
		class UProjectileAsset* ProjectileAsset;
	
	/***************/
	/*bullet spread*/
	/***************/

	UPROPERTY(Replicated)
		uint32 BulletSpreadSeed;
	/*recoil spread pattern that can be simulated accurately*/
	FRandomStream BulletSpreadStream;

	UPROPERTY()
		float CurrentBulletSpread;
	UPROPERTY(EditDefaultsOnly, Category = "Aiming|Recoil", meta = (UIMin=0, ClampMin=0,UIMax=100,ClampMax=100))
		float MaxBulletSpread;
	UPROPERTY(EditDefaultsOnly, Category = "Aiming|Recoil", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100))
		float MinBulletSpread;
	/*amount per second to reach max recoil*/
	UPROPERTY(EditDefaultsOnly, Category = "Aiming|Recoil", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100))
		float BulletSpreadRate;
	/*how long after last shot does it take before spread reduction occurs*/
	UPROPERTY(EditDefaultsOnly, Category = "Aiming|Recoil", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100))
		float SpreadReductionDelay;
	/*amount applied per second after Delay finishes*/
	UPROPERTY(EditDefaultsOnly, Category = "Aiming|Recoil", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100))
		float SpreadReductionRate;
	/*penalty amount applied for walking */
	UPROPERTY(EditDefaultsOnly, Category = "Aiming|Recoil", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100))
		float MovementSpreadAmount;
	UPROPERTY()
		float MovementSpreadRate;


	/*aiming*/

	
public:
	//how long it takes to switch to aim down sights
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Aiming")
		float AimDownSightsTime;
	//Arms/Weapon Offset to assist with aligning
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Aiming")
		FVector AimOffset;

	/*audio/sound*/
protected:
	UPROPERTY()
		class UAudioComponent* FireAudioComponent;
	UPROPERTY()
		class UAudioComponent* DryFireAudioComponent;
	UPROPERTY()
		class UAudioComponent* TailFireAudioComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta=(EditCondition=bHasSemiFireMode))
		class USoundCue* SemiFireSound;
	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta=(EditCondition=bHasBurstFireMode))
		class USoundCue* BurstFireSound;
	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (EditCondition=bHasFullAutoMode))
		class USoundCue* AutoFireSound;

	//how long it takes to fade out the last shot
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		float FireFadeOutDuration;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		class USoundCue* FireTailSound;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		float TailFadeOutDuration;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		class USoundCue* DryfireSound;

	/*collision*/
protected:
	UPROPERTY(BlueprintReadOnly)
		bool bMuzzleBlocked;  //whether our muzzle is colliding with something
	UPROPERTY()
		float MuzzleBlockTraceLength;

	/*animations*/
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
		class UCurveVector* MovementSwayCurve;
	/*how much to drop the weapon when walking forward*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
		float ForwardMovementScale;

	/*@DEPRECIATED : TO BE REMOVED*/
protected:
	UPROPERTY()		class UAnimSequenceBase* ArmsFireAnimation;
	UPROPERTY()		class UAnimSequenceBase* WeaponFireAnimation;
	UPROPERTY()		class UAnimSequenceBase* ArmsADSFireAnimation;
	UPROPERTY()		class UAnimSequenceBase* ArmsReloadEmptyAnimation;
	UPROPERTY()
		class UAnimSequenceBase* WeaponReloadEmptyAnimation;



	
	/*fx*/
protected:
	UPROPERTY()
		class UNiagaraComponent* MuzzleFlashComponent;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Muzzle Flash")
		class UNiagaraSystem* MuzzleFlash;
	UPROPERTY(VisibleDefaultsOnly)
		FName MuzzleSocketName = "MuzzleFlashSocket";




	//=============================================================================================
	//==========================================FUNCITONS==========================================
	//=============================================================================================

	AShooterWeapon(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime);

	virtual void BeginPlay() override;

//==========================================
//==================FIRING==================
//==========================================
public:
	virtual void BeginFire() override;
	virtual void EndFire() override;
	virtual void ServerNotifyBeginFire_Implementation() override;
	virtual void ServerNotifyEndFire_Implementation() override;
	virtual bool CanFire() override;
	virtual bool IsFiring() override;

protected:
	UFUNCTION()
		virtual void PerformFire();
	//repeats firing cycle automatically when in burst/auto mode
	UFUNCTION()
		void InitFireLoopTimer();
	UFUNCTION()
		void PerformShot();
	
	//============================================
	//===================RECOIL===================
	//============================================
protected:
	UFUNCTION()
		virtual void CalcRecoil(float DeltaTime);
	UFUNCTION()
		virtual void ApplyRecoil(FVector& FireDirection);
private:
	UFUNCTION()
		void RandomizeFireSeed();
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSetFireSeed(int32 NewSeed);
public:
	/*CurrentRecoil / MaxRecoil (0.0f - 1.0f)*/
	UFUNCTION(BlueprintPure, Category = "Recoil")
		float ReturnBulletSpreadPercentage();
	//===========================================
	//================PROJECTILES================
	//===========================================
public:
	UFUNCTION()
		class AProjectileManager* ReturnProjectileManager();
protected:
	UFUNCTION()
		virtual FProjectileTrajectory ReturnProjectileTrajectory();
public:
	UFUNCTION()
		FVector ReturnMuzzleLocation();

	//======================================
	//=================AMMO=================
	//======================================
protected:
	UFUNCTION()
		virtual void ConsumeAmmo();
public:
	/*if we have any ammo in our chambers/magazines to fire*/
	UFUNCTION(BlueprintPure, Category = "Ammunition")
		virtual bool HasAmmo();
	UFUNCTION(BlueprintPure, Category = "Ammunition")
		virtual int32 ReturnTotalAmmoForWeapon();
	UFUNCTION(BlueprintPure, Category = "Ammunition")
		virtual int32 ReturnMagazinesOnSpawn();
	UFUNCTION(BlueprintPure, Category = "Ammunition")
		virtual int32 ReturnAmmoCountOnSpawn();
	UFUNCTION(BlueprintPure, Category = "Ammunition")
		virtual class UProjectileAsset* ReturnAmmoClass();
	//========================================
	//================FIRERATE================
	//========================================
public:
	UFUNCTION(BlueprintCallable, Category = "Fire Rate")
		float ReturnFireRate();

	
	//======================================
	//================AIMING================
	//======================================
public:
	UFUNCTION()
		void AimDownSights();
	UFUNCTION()
		void ReleaseSights();
	UFUNCTION()
		float ReturnDesiredFOV();

	//======================================
	//================RELOAD================
	//======================================
public:
	UFUNCTION(BlueprintPure, Category = "Reloading")
		virtual bool CanReload();
	UFUNCTION()
		virtual void Reload();
protected:
	UFUNCTION()
		virtual void BeginReload();

public:
	UFUNCTION()
		virtual bool CanIncrementRound();
protected:
	UFUNCTION()
		virtual void IncrementRound();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerNotifyIncrementRound();
public:
	UFUNCTION(BlueprintPure, Category = "Reloading")
		virtual bool IsReloading();
	/*returns percentage of reload progress completed (range 0.0f to 1.0f)*/
	UFUNCTION(BlueprintPure, Category = "Reloading")
		virtual float ReturnReloadProgress();
protected:
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerNotifyReload();
public:
	UFUNCTION()
		virtual void OnReloadKeyReleased();
	UFUNCTION(BlueprintCallable, Category = "Reloading")
		virtual void EndReload();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerNotifyEndReload();
protected:
	UFUNCTION()
		virtual void OnReloadFinished();

	//=================================
	//============EQUIPPING============
	//=================================
	virtual void OnEquippedFinished() override;

	//=====================================
	//================AUDIO================
	//=====================================
protected:
	virtual	void MakeNoise(float Loudness, APawn* NoiseInstigator, FVector NoiseLocation, float MaxRange, FName Tag);
	UFUNCTION()
		virtual void PlayFireSound();
	UFUNCTION()
		virtual void EndFireSound();
	UFUNCTION()
		virtual void PlayDryfireSound();

	//========================================
	//===============ANIMATIONS===============
	//========================================
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "NotifyReloadAnimFinished"))
		virtual void OnAnimNotifyReloadFinished();
	UFUNCTION(BlueprintCallable, meta=(DisplayName="NotifyInsertShellAnimFinished"))
		virtual void OnAnimNotifyInsertShell();	
protected:
	UFUNCTION()
		virtual void PlayRecoilAnimation();
	UFUNCTION()
		virtual void PlayReloadAnimation();


	//=====================================
	//==============COLLISION==============
	//=====================================
protected:
	/*determines if the Muzzle is getting blocked*/
	UFUNCTION()
		virtual void CalcMuzzleCollision();




	//==================================
	//================FX================
	//==================================
protected:
	UFUNCTION()
		virtual void PlayMuzzleFlash();
};

