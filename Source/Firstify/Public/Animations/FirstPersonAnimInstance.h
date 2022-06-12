// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimInstance.h"
#include "Players/FirstifyCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/MeleeWeapon.h"
#include "Weapons/ShooterWeapon.h"
#include "FirstPersonAnimInstance.generated.h"




/**
 * 
 */
UCLASS(abstract, Blueprintable, meta=(BlueprintThreadSafe))
class FIRSTIFY_API UFirstPersonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
private:


	//=======================
	//=======CHARACTER=======
	//=======================
	UPROPERTY()
		AFirstifyCharacter* Character;


	//==================
	//========IK========
	//==================
	UPROPERTY(EditAnywhere, Category = "Preview Only|IK", meta = (UIMin = -1.0, UIMax = 1.0, ClampMin = -1.0, ClampMax = 1.0))
		float RightHandIKAlpha;
	UPROPERTY(EditAnywhere, Category = "Preview Only|IK", meta = (UIMin = -1.0, UIMax = 1.0, ClampMin = -1.0, ClampMax = 1.0))
		float LeftHandIKAlpha;


	//======================
	//=======MOVEMENT=======
	//======================
	UPROPERTY(EditAnywhere, Category = "Preview Only|Movement", meta=(UIMin=-1.0,UIMax=1.0,ClampMin=-1.0,ClampMax=1.0))
		float RotationRate;
	UPROPERTY(EditAnywhere, Category = "Preview Only|Movement", meta = (UIMin = -1.0, UIMax = 1.0, ClampMin = -1.0, ClampMax = 1.0))
		float PitchRate;
	



	//=======================
	//========WEAPONS========
	//=======================
	UPROPERTY(VisibleAnywhere, Category = "Animation Montages|Weapons")
		FName FireSlotName;
	UPROPERTY(EditAnywhere, Category = "Animation Montages|Weapons")
		class UAnimSequenceBase* SemiFireAnimation;

	UPROPERTY(EditAnywhere, Category = "Preview Only|Weapons")
		class AWeapon* Weapon;
	UPROPERTY(EditAnywhere, Category = "Preview Only|Weapons")
		bool bWeaponInHand;
	UPROPERTY(EditAnywhere, Category = "Preview Only|Weapons")
		bool bWeaponFiring;
	UPROPERTY(EditAnywhere, Category = "Preview Only|Weapons")
		bool bAimingDownSights;
	

	UPROPERTY(EditAnywhere, Category = "Preview Only|Weapons")
		EWeaponState WeaponState;

	
	
	

		
		
	//===================================================================================================
	//=============================================FUNCTIONS=============================================
	//===================================================================================================
public:
	
	UFirstPersonAnimInstance();
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;
	


	
	//=================================
	//============CHARACTER============
	//=================================
	UFUNCTION(BlueprintPure, Category = "Character")
		AFirstifyCharacter* ReturnCharacter() { return Cast<AFirstifyCharacter>(TryGetPawnOwner()); };

	

	//==================================
	//=============MOVEMENT=============
	//==================================
	UFUNCTION()
		void UpdateMovementState(float DeltaTimeX);
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Movement")
		float ReturnPitchRate() {return PitchRate;};
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Movement")
		float ReturnRotationRate() { return RotationRate; };


	//=================================
	//=============WEAPONS=============
	//=================================
	/*core tick-update*/
	UFUNCTION()
		void UpdateWeaponState();
	/*anim-events*/
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AnimNotifies")
		void NotifyWeapon_ReloadComplete();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AnimNotifies")
		void NotifyWeapon_InsertShell();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AnimNotifies")
		void NotifyWeapon_EquipComplete();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AnimNotifies", meta = (DisplayName="Notify Weapon UnEquip Complete"))
		void NotifyWeapon_UnEquipComplete();
	/*animation-montages*/
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Animation Montages")
		void PlayWeaponFireAnimation();
	/*getters*/
	UFUNCTION(BlueprintPure, Blueprintcallable, Category = "Weapons")
		bool HasWeaponInHand() { return bWeaponInHand; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapons")
		class AWeapon* ReturnWeapon(){ return Weapon; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapons")
		class AWeapon* ReturnWeaponAsShooter(){ return Cast<AShooterWeapon>(Weapon); };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapons")
		class AWeapon* ReturnWeaponAsMelee() { return Cast<AMeleeWeapon>(Weapon); };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapons")
		EWeaponState ReturnWeaponState() { return WeaponState; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapons")
		bool IsAimingDownSights() { return bAimingDownSights; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapons")
		bool IsWeaponFiring() { return bWeaponFiring; };
	




	//===============================
	//===========UTILITIES===========
	//===============================
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Animation")
		class AFirstifyCharacter* TryGetFirstifyPawnOwner() { return Cast<AFirstifyCharacter>(TryGetPawnOwner()); }
};
