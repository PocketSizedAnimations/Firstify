// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Animations/FirstPersonAnimInstance.h"
#include "Weapons/Weapon.h"
#include "Weapons/ShooterWeapon.h"
#include "Weapons/MeleeWeapon.h"
#include "Players/FirstifyCharacter.h"

UFirstPersonAnimInstance::UFirstPersonAnimInstance()
{
	/*movement*/
	PitchRate = 0.0f;
	RotationRate = 0.0f;

	/*weapons*/
	bWeaponInHand = false;
	WeaponState = EWeaponState::Equipped;
	bWeaponFiring = false;
	bAimingDownSights = false;

	/*weapons - animations*/
	FireSlotName = FName("Firing");
	
}

void UFirstPersonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UFirstPersonAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);

	/*editor-previewer hack*/
	if (!TryGetFirstifyPawnOwner())
		return;

	/*runtime updates*/
	UpdateMovementState(DeltaTimeX);
	UpdateWeaponState();
}


/*UpdateMovementState() - Updates all variables in relation to player movement*/
void UFirstPersonAnimInstance::UpdateMovementState(float DeltaTimeX)
{
	if (!ReturnCharacter())
		return;

	float PreviousPitchRate = PitchRate;
	float CurrentPitchRate = FMath::Clamp(ReturnCharacter()->PitchRate / 3, -1.0f, 1.0f);
	PitchRate = FMath::FInterpTo(PreviousPitchRate, CurrentPitchRate, DeltaTimeX, 3.0f);;
	//PitchRate = FMath::Lerp(PreviousPitchRate, CurrentPitchRate, 1.0f);

	
	
	float PreviousRotationRate = RotationRate;
	float CurrentRotationRate = FMath::Clamp(ReturnCharacter()->RotationRate / 3, -1.0f, 1.0f);
	RotationRate = FMath::FInterpTo(PreviousRotationRate, CurrentRotationRate, DeltaTimeX, 3.0f);
	//RotationRate = FMath::Lerp(PreviousRotationRate, CurrentRotationRate, 1.0f);
}


//=================================
//=============WEAPONS=============
//=================================


/*UpdateWeaponState() - Updates all variables in relation to weapons
*
*
*
*
*/
void UFirstPersonAnimInstance::UpdateWeaponState()
{	
	if (!ReturnCharacter() || !ReturnCharacter()->ReturnCurrentWeapon())
		return;

	bWeaponInHand = TryGetFirstifyPawnOwner()->HasWeaponInHand();
	Weapon = ReturnCharacter()->ReturnCurrentWeapon();
	WeaponState = Weapon->WeaponState;
	bWeaponFiring = Weapon->IsFiring();
	bAimingDownSights = ReturnCharacter()->IsAimingDownSights();
}


void UFirstPersonAnimInstance::NotifyWeapon_ReloadComplete()
{
	if (!Weapon || !Weapon->IsA<AShooterWeapon>())
		return;

	Cast<AShooterWeapon>(Weapon)->OnAnimNotifyReloadFinished();
}

void UFirstPersonAnimInstance::NotifyWeapon_InsertShell()
{
	if (!Weapon || !Weapon->IsA<AShooterWeapon>())
		return;

	Cast<AShooterWeapon>(Weapon)->OnAnimNotifyInsertShell();
}

void UFirstPersonAnimInstance::NotifyWeapon_EquipComplete()
{
	if (!Weapon)
		return;

	Weapon->OnAnimNotifyEquipFinished();
}

void UFirstPersonAnimInstance::NotifyWeapon_UnEquipComplete()
{
	if (!Weapon)
		return;

	Weapon->OnAnimNotifyUnequipFinished();
}

void UFirstPersonAnimInstance::PlayWeaponFireAnimation()
{
	if (!SemiFireAnimation)
		return;

	PlaySlotAnimationAsDynamicMontage(SemiFireAnimation, FireSlotName,0.01f,0.05f);
}

