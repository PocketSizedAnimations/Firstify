// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryManager/InventoryItemComponent.h"
#include "FirstifyCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeath, APawn*, PawnKilled, AController*, Instigator, AActor*, DamageDealer);

UENUM(BlueprintType)
enum class EHealthState : uint8
{
	Healthy,
	Injured,
	Incapacitated,
	Dead
};

USTRUCT(BlueprintType)
struct FBodyPartCondition
{
	GENERATED_BODY();
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Part Condition")
		float DamagePercent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Part Condition")
		bool bBleeding;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Part Condition")
		bool bFractured;
};

USTRUCT(BlueprintType)
struct FBodyCondition
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition Head;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition Chest;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition Abdomen;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition LeftArm;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition RightArm;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition LeftLeg;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body Condition")
		FBodyPartCondition RightLeg;
};

UENUM(BlueprintType)
enum class EBodyPart : uint8
{
	Head,
	Neck,
	Chest,
	Back,
	Abdomen,
	LowerBack,
	Groin,
	Butt,
	LeftArm,
	LeftWrist,
	LeftHand,
	RightArm,
	RightWrist,
	RightHand,
	LeftThigh,
	LeftCalf,
	LeftFoot,
	RightThigh,
	RightCalf,
	RightFoot,
	Unknown
};

UENUM()
enum class ESide : uint8
{
	Center,
	Left,
	Right	
};

UENUM(BlueprintType)
enum class ETurnDirection : uint8
{
	None,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FFootstepSound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PhysicalProperties)
		TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physical Properties")
		class USoundCue* Sound;
};

UCLASS(abstract)
class FIRSTIFY_API AFirstifyCharacter : public ACharacter
{
	GENERATED_BODY()
public:
		
		/*weapons*/
protected:
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Aiming")
		bool bAimingDownSights;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Reloading")
		bool bReloadKeyHeld;
protected:
	FTimerHandle ADSHandler;
		
		/*inventory*/
public:
	UPROPERTY(BlueprintReadOnly,VisibleAnywhere, Category = "Inventory")
		class UInventoryManagerComponent* InventoryManager;
protected:
	UPROPERTY(Replicated, ReplicatedUsing = "OnRep_InHandChanged")
		AActor* InHand;
	UPROPERTY()
		bool bInHandRepChange;

private:
	//item we're currently trying to equip
	UPROPERTY()
		AActor* PendingInHand;
	/*interactives*/
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Interactives")
		AActor* CurrentInteractive;
public:
	UPROPERTY(EditAnywhere, Category = "Interactives", meta = (DisplayName = "Draw Debug Info"))
		bool bDebugInteractives;
protected:
	UPROPERTY(EditAnywhere, Category = "Interactives")
		float InteractiveTraceLength;
		
	/*health*/
	UPROPERTY(EditAnywhere, Replicated, ReplicatedUsing="OnRep_HealthUpdate",BlueprintReadOnly, Category = "Health")
		float Health;
private:
	//what our health was last update - used for Health update checks
	UPROPERTY()
		float LastHealthUpdate;
protected:
	//Healthy/Injured/Incapacitated/Dead
	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Health")
		EHealthState HealthState;
	//Individual Body Part Conditions
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Health|BodyCondition")
		FBodyCondition BodyCondition;
	//helper reference storing last place we were hit
	UPROPERTY()
		EBodyPart LastHitBodyPart;
	/*list of bone-names for IDing what body part we hit*/
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> HeadBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> NeckBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> TorsoBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> AbdomenBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> PelvisBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> LeftArmBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> RightArmBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> LeftLegBones;
	UPROPERTY(EditDefaultsOnly, Category = "Health|Body|Bones")
		TArray<FName> RightLegBones;

	/*useful UI delegates for health*/
	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthChange OnHealthIncrease;
	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnHealthChange OnHealthDecrease;


public:
	/*damage effects*/
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Bullet|Small")
		class UNiagaraSystem* BulletImpactFX_Small;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Bullet|Small")
		TArray<class UMaterialInstance*> BloodSplatters_Small;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Bullet|Medium")
		class UNiagaraSystem* BulletImpactFX_Medium;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Bullet|Medium")
		TArray<class UMaterialInstance*> BloodSplatters_Medium;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Bullet|Large")
		class UNiagaraSystem* BulletImpactFX_Large;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Bullet|Medium")
		TArray<class UMaterialInstance*> BloodSplatters_Large;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Dismembered")
		class UNiagaraSystem* DismemberedFX;
	UPROPERTY(EditDefaultsOnly, Category = "FX|Damage|Exploded")
		class UNiagaraSystem* ExplodedFX;


public:
	/*death*/
	UPROPERTY(BlueprintAssignable, Category = "Health")
		FOnDeath OnDeathEvent;
	UPROPERTY(EditDefaultsOnly, Category = "Death")
		bool bFadeCameraOnDeath;
	UPROPERTY(EditDefaultsOnly, Category = "Death")
		bool bAutoSpectatateOnDeath;
	FTimerHandle DeathScreenFadeTimer;


	/*movement*/
public:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		ETurnDirection TurnDirection;
	/*whether or not we're currently on the initial keystroke to crouch*/
	bool bInitialCrouchKeystroke;

	/*hud*/
protected:
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
		class TSubclassOf<UUserWidget> HUDClass;

	//-----------------------------
	//------------AUDIO------------
	//-----------------------------
protected:
	UPROPERTY()
		class UAudioComponent* AudioComponent;
	/*footsteps*/
	UPROPERTY(EditDefaultsOnly, Category = "Sounds|Footsteps|Sockets")
		FName LeftFootSocketName;
	UPROPERTY(EditDefaultsOnly, Category = "Sounds|Footsteps|Sockets")
		FName RightFootSocketName;
	UPROPERTY()
		class USoundCue* GenericFootstep;
	UPROPERTY(EditDefaultsOnly, Category = "Sounds|Footsteps")
		class UFootstepSounds* SurfaceFootstepSounds;
	UPROPERTY(EditDefaultsOnly, Category = "Sounds|Footsteps")
		class UFootstepSounds* SurfaceLandSounds;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds|Movement")
		class USoundCue* GenericLandedSound;

	//UPROPERTY(EditDefaultsOnly, Category = "Sounds|Pain")
	//	class USoundCue* GenericPainSound;
	//UPROPERTY(EditDefaultsOnly, Category = "Sounds|Pain")
	//	class USoundCue* FallPainSound;
	//UPROPERTY(EditDefaultsOnly, Category = "Sounds|Pain")
	//	class USoundCue* TerminalFallSound;
	UPROPERTY(EditDefaultsOnly, Category = "Sounds|Pain")
		class UPainSounds* PainSounds;

	/*animations*/
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animations|Death Animations")
		TArray<class UCharacterDeathAnimations*> DeathAnimationSets;
	FTimerHandle DeathAnimationTimer;
	//Socket we attach the First Person Camera to on death
	UPROPERTY(EditAnywhere, Category = "Animations|Death Animations")
		FName FirstPersonDeathSocketName;


//------------------------------------------------
//---------------------CAMERA---------------------
//------------------------------------------------
public:
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
		bool bFreelookEnabled;
	/*character pitch*/
	UPROPERTY(Replicated, ReplicatedUsing="OnRep_PitchAmount", BlueprintReadOnly, Category = "Camera")
		float PitchAmount; 
private:
	UPROPERTY()
		float LastPitchAmount;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
		float PitchRate;

	/*character rotation*/
private:
	UPROPERTY()
		float RotationAmount;
	UPROPERTY()
		float LastRotationAmount;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
		float RotationRate;

	UPROPERTY()
		FRotator LastRotation;



	/*camera setup*/	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	//	class USceneComponent* FirstPersonScene;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class USpringArmComponent* CameraSpringComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		class UFirstifyCameraComponent* CameraComponent;
	/*the mesh to use for first person arms*/

	/*************************/
	/******skeletal mesh*****/
	/*************************/

	UPROPERTY(VisibleAnywhere)
		FName RightHandSocketName;
	UPROPERTY(VisibleAnywhere)
		FName LeftHandSocketName;

	//-------------------------------
	//-------FIRST PERSON ARMS-------
	//-------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "First Person")
		class UFirstPersonArmsComponent* FirstPersonArms;
	UPROPERTY()
		TSubclassOf<UAnimInstance> DefaultFirstPersonArmsAnimBlueprint;


	/*the recorded default offset from BeginPlay()*/
	UPROPERTY()
		FVector ArmsDefaultCameraOffset;

	/*cheats*/
	UPROPERTY()
		bool bGodMode;

	//===================================================================================================================
	//=====================================================FUNCTIONS=====================================================
	//===================================================================================================================
public:
	// Sets default values for this character's properties
	AFirstifyCharacter(const FObjectInitializer& ObjectInitializer);

	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//==========================================
	//================POSSESSION================
	//==========================================
public:
	virtual void PossessedBy(AController* NewController);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//===================================
	//==============WEAPONS==============
	//===================================
protected:
	UFUNCTION(BlueprintCallable, Category = "In Hand")
		virtual void BeginFire();
	UFUNCTION(BlueprintCallable, Category = "In Hand")
		virtual void EndFire();
	UFUNCTION()
		virtual bool CanFire();
	UFUNCTION()
		virtual bool CanAimDownSights();
	UFUNCTION()
		virtual void AimDownSights();
	UFUNCTION()
		virtual void AimDownSightsLERP();
	UFUNCTION()
		virtual void ReleaseSights();

	//===================================
	//=============INVENTORY=============
	//===================================

public:
	UFUNCTION()
		virtual void Equip(AActor* Item);
	UFUNCTION()
		virtual void EquipPending();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerNotifyEquip(AActor* Item);
	UFUNCTION()
		virtual void OnEquippedFinished(AActor* Item);
	UFUNCTION()
		virtual void UnEquip(AActor* Item);
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerNotifyUnEquip(AActor* Item);
	UFUNCTION()
		virtual void OnUnEquippedFinished(AActor* Item);
	UFUNCTION()
		virtual void OnRep_InHandChanged();
	UFUNCTION()
		virtual void SwitchToPrimary();
	UFUNCTION()
		virtual void SwitchToAlternative();
	UFUNCTION()
		virtual void SwitchToSecondary();
protected:
	UFUNCTION()
		virtual void Reload();
	UFUNCTION()
		virtual void ReloadReleased();

public:
	UFUNCTION()
		UInventoryItemComponent* ReturnItemComponent(AActor* Item);


protected:
	/*just a bunch of basic functions for InputComponent to be able to call*/
	UFUNCTION()
		virtual void Quickslot1();
	UFUNCTION()
		virtual void Quickslot2();
	UFUNCTION()
		virtual void Quickslot3();
	UFUNCTION()
		virtual void Quickslot4();
	UFUNCTION()
		virtual void Quickslot5();
	UFUNCTION()
		virtual void Quickslot6();
	UFUNCTION()
		virtual void Quickslot7();
	UFUNCTION()
		virtual void Quickslot8();
	UFUNCTION()
		virtual void Quickslot9();


public:
	/*instantly uses the item within the specific quickSlot*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		virtual void  QuickUse(EQuickslot Slot)
	{
		if (InventoryManager && InventoryManager->ReturnQuickslotItem(Slot))
		{
			if (UInventoryItemComponent* ItemComp = InventoryManager->ReturnItemComponent(InventoryManager->ReturnQuickslotItem(Slot)))
				ItemComp->QuickUse(this);
		}
	}

	//=============================
	//===========IN HAND===========
	//=============================
public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
		FORCEINLINE	class AActor* ReturnInHand() { return InHand; }
	
	template <class T>
	T* ReturnInHand() const
	{
		return Cast<T>(InHand);
	}
	UFUNCTION()
		void ClearInHand();

	UFUNCTION(BlueprintCallable, Category = "In Hand")
		virtual void DropInHand();

	//===================================
	//==============WEAPONS==============
	//===================================
public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
		virtual bool HasWeaponInHand();
	UFUNCTION(BlueprintPure, Category = "Inventory")
		virtual class AWeapon* ReturnCurrentWeapon();
	UFUNCTION(BlueprintPure, Category = "Inventory")
		virtual class AShooterWeapon* ReturnCurrentShooterWeapon();
	UFUNCTION(BlueprintPure, Category = "Inventory")
		virtual class AMeleeWeapon* ReturnCurrentMeleeWeapon();
	UFUNCTION(BlueprintPure, Category = "Weapons")
		virtual bool IsAimingDownSights();

public:
	UFUNCTION(BlueprintCallable, Category = "Weapons")
		virtual void ReturnFireTrajectory(FVector& OutLocation, FVector& OutDirection);
	UFUNCTION(exec)
		virtual void DebugTest();

	//====================================
	//============INTERACTIVES============
	//====================================
protected:
	UFUNCTION()
		virtual void CalcInteractives();
	UFUNCTION()
		virtual bool IsInteractive(AActor* InteractiveObject);
	UFUNCTION()
		virtual void Use();
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerRequestUse(AActor* InteractiveObject);
	UFUNCTION(BlueprintCallable, Category = "Interactives")
		virtual void PickupItem(AActor* InteractiveActor);
	UFUNCTION(BlueprintCallable, Category = "Interactives")
		virtual void ReleaseGrabbedItem();
	
	//================================
	//============MOVEMENT============
	//================================
public:
	UFUNCTION(BlueprintPure, Category = "Movement")
		UFirstifyCharacterMovement* ReturnCharacterMovement();

protected:
	UFUNCTION()
		virtual void Pitch(float Value);
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerUpdatePitch(float NewPitch);
	UFUNCTION()
		virtual void OnRep_PitchAmount();
	UFUNCTION()
		virtual void Rotate(float Value);
	UFUNCTION()
		virtual void UpdateRotationRate();
	UFUNCTION()
		virtual bool CanMove();
	UFUNCTION()
		virtual void MoveForward(float Value);
	UFUNCTION()
		virtual void MoveRight(float Value);

	UFUNCTION()
		virtual void OnCrouchPressed();
		virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
		virtual void LerpCrouch();
		FTimerHandle CrouchHandler;
	UFUNCTION()
		virtual void OnCrouchReleased();
		virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UFUNCTION()
		virtual void Sprint();
	UFUNCTION()
		virtual void StopSprinting();
	UFUNCTION()
		virtual bool CanSprint();

	
	
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void Landed(const FHitResult& Hit) override;



	
	//=======================================
	//============DAMAGE && DEATH============
	//=======================================
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
protected:
	virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	
	UFUNCTION(NetMulticast, Unreliable)
		virtual void MulticastOnTakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
	UFUNCTION()
		virtual void OnRep_HealthUpdate();

protected:
	UFUNCTION()
		virtual float TakeHeadDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent);
	UFUNCTION()
		virtual float TakeNeckDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent);
	UFUNCTION()
		virtual float TakeChestDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent);
	UFUNCTION()
		virtual float TakeAbdominalDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent);
	UFUNCTION()
		virtual float TakePelvicDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent);
	UFUNCTION()
		virtual float TakeArmDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent, ESide Side);
	UFUNCTION()
		virtual float TakeLegDamage(float DamageAmount, struct FPointDamageEvent const& PointDamageEvent, ESide Side);

	UFUNCTION()
		virtual float TakeFallDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Health")
		virtual bool IsPendingDeath();
public:
	UFUNCTION(BlueprintPure, Category = "Health")
		virtual bool IsDeadOrIncapacitated();
	UFUNCTION(BlueprintPure, Category = "Health")
		virtual bool IsDead();
protected:
	//kills the player and calls OnDeath events
	UFUNCTION()
		virtual void Die(float Damage, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
	//replicated for all players - performs any death animation and local logic needed
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastOnDeath(class AController* EventInstigator, AActor* DamageCauser, TSubclassOf<class UDamageType> DamgeType, EBodyPart BodyPartHit = EBodyPart::Unknown, class UAnimSequenceBase* DeathAnimation = nullptr);
	//notifier for when all death animations && logic has completed
	UFUNCTION()
		virtual void OnDeathComplete();
	UFUNCTION()
		virtual void BeginDeathScreenFade();
	UFUNCTION()
		virtual void OnEndDeathScreenFade();
public:
	UFUNCTION()
		virtual void Suicide(AController* EventInstigator);


	UFUNCTION()
		virtual class UAnimSequenceBase* ReturnDeathAnimation(float Damage = 0.0f,EBodyPart BodyPartHit = EBodyPart::Unknown, FDamageEvent const& DamageEvent = FDamageEvent(), class AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	UFUNCTION()
		virtual void Ragdoll();

	//=====================================
	//==============DAMAGE FX==============
	//=====================================
public:
	//UFUNCTION(BlueprintPure, Category = "Damage|FX")
	//	UMaterialInstance* ReturnBloodSplatterDecal(EImpactFXSize ImpactSize);


	//=================================
	//===============HUD===============
	//=================================
public:
	UFUNCTION(BlueprintPure, Category = "HUD")
		virtual TSubclassOf<class UUserWidget> ReturnHUDClass();




	//==================================
	//==============CAMERA==============
	//==================================
protected:
	UFUNCTION()
		virtual void ToggleFreelook();
	UFUNCTION()
		virtual void EnableFreelook();
	UFUNCTION()
		virtual void DisableFreelook();


	//============================================
	//============POST PROCESS EFFECTS============
	//============================================
public:
	UFUNCTION(BlueprintCallable, Category = "Camera")
		void AddOrUpdatePostProcessEffect(class UMaterialInterface* PostProcessMaterial, float Weight = 1.0f);

	//=================================
	//==========SKELETAL MESH==========
	//=================================
public:
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastAttachToRightHand(AActor* ItemToAttach);
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastAttachToLeftHand(AActor* ItemToAttach);

	//====================================
	//=============ANIMATIONS=============
	//====================================
public:
	UFUNCTION()
		class UAnimInstance* ReturnFirstPersonAnimTree();
	
	UFUNCTION()
		virtual void InitFirstPersonArms(TSubclassOf<class UFirstPersonAnimInstance> ArmsAnimBlueprint);
	UFUNCTION()
		virtual void ResetFirstPersonArms();

	//==================================
	//==============SOUNDS==============
	//==================================
public:
	UFUNCTION(BlueprintCallable, Category = "Audio")
		virtual void PlayFootstepSound(ESide Foot);
	UFUNCTION(BlueprintCallable, Category = "Audio")
		virtual void PlayPainSound(TSubclassOf<UDamageType> DamageType);
	//UFUNCTION()
	//	virtual void PlayGenericPainSound();
	UFUNCTION()
		virtual void PlayLandedSound();
	//UFUNCTION()
	//	virtual void PlayFallPainSound();
	//UFUNCTION()
	//	virtual void PlayTerminalFallSound();

	//=======================================
	//===============UTILITIES===============
	//=======================================
public:
	virtual class AFirstifyPlayerController* ReturnPlayerController();

	//======================================
	//===========DEINITIALIZAITON===========
	//======================================
	virtual void BeginDestroy() override;
	virtual void Destroyed() override;

};
