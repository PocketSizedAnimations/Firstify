// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Weapons/PrimaryWeaponInterface.h"
#include "Weapons/SecondaryWeaponInterface.h"

/*animations*/
#include "Animations/FirstPersonAnimInstance.h"

/*components*/
#include "Animations/Components/FirstPersonArmsComponent.h"

/*debugging*/
#include "Firstify.h"
#include "Engine/Engine.h"

/*animations*/
#include "Animation/AnimInstance.h"
#include "Animations/FirstPersonAnimInstance.h"  

#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

/*inventory && interaction*/
#include "InventoryManager/InventoryManagerComponent.h"
#include "InventoryManager/InventoryItemComponent.h"
#include "Interactives/InteractiveComponent.h"

#include "Players/FirstifyCharacter.h"

/*replication*/
#include "Net/UnrealNetwork.h"

/*timers*/
#include "TimerManager.h"

/*setup replication*/
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, bFiring, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, WeaponState, COND_SkipOwner);
}

// Sets default values
AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
{
	/*configuration*/
	WeaponType = EWeaponType::Firearm;
	WeaponState = EWeaponState::UnEquipped;
	
	
	/*rendering*/
	SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
	SetRootComponent(SceneComponent);

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh3PComp"));
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh1PComp"));	

	if (Mesh1P)
	{
		Mesh1P->SetupAttachment(SceneComponent);
		Mesh1P->bOnlyOwnerSee = true;
		Mesh1P->SetCollisionProfileName("None", false);
		Mesh1P->bReceivesDecals = false;
	}
	if (Mesh3P)
	{
		Mesh3P->SetupAttachment(SceneComponent);
		Mesh3P->bOwnerNoSee = true;
		Mesh3P->SetCollisionProfileName("Weapon");
		Mesh3P->bReceivesDecals = false;
	}
	
	/*inventory item setup*/
	InventoryItem = ObjectInitializer.CreateDefaultSubobject<UInventoryItemComponent>(this, TEXT("InventoryItemComp"));

	if (InventoryItem)
	{
		InventoryItem->bAllowQuickslotting = false;
		InventoryItem->QuickslotGroup = EQuickslot::None;
		InventoryItem->bCanPickup = true;
		

		//InventoryItem->OnItemEquipped.AddDynamic(this, &AWeapon::OnEquipped);
		//InventoryItem->OnItemUnequipped.AddDynamic(this, &AWeapon::OnUnequipped);
		InventoryItem->OnItemPickup.AddDynamic(this, &AWeapon::OnPickup);
	}

	//InteractiveComponent = ObjectInitializer.CreateDefaultSubobject<UInteractiveComponent>(this, TEXT("InteractiveComp"));

	//if (InteractiveComponent)
	//{
	//	InteractiveComponent->bIsInteractive = true;
	//}

	/*replication*/
	bReplicates = true;
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();


	/*initiate mesh*/
	Mesh3P->SetVisibility(true);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//=========================================
//=============WEAPON HANDLING=============
//=========================================

void AWeapon::BeginFire()
{

}

void AWeapon::EndFire()
{

}

void AWeapon::OnRep_FiringChange()
{
	if(bFiring)
		BeginFire();
	else
		EndFire();
}

bool AWeapon::IsFiring()
{
	return bFiring;
}

bool AWeapon::CanFire()
{
	if (WeaponState != EWeaponState::Equipped)
		return false;
	
	return true;
}

bool AWeapon::ServerNotifyBeginFire_Validate()
{
	return true;
}

void AWeapon::ServerNotifyBeginFire_Implementation()
{

}

bool AWeapon::ServerNotifyEndFire_Validate()
{
	return true;
}

void AWeapon::ServerNotifyEndFire_Implementation()
{

}

//========================================
//===========INVENTORY HANDLING===========
//========================================

void AWeapon::OnEquipped(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s%s::OnEquipped(%s)"),NETMODE_WORLD, *GetName(),*GetNameSafe(User));

	/*prevent us from re-equipping this weapon while already in the process*/
	if (WeaponState == EWeaponState::Equipping || WeaponState == EWeaponState::UnEquipping)
		return;

	/*attach 1st person*/	
	AttachToFirstPersonArms();

	/*switch state*/
	SetState(EWeaponState::Equipping);

	/*disable collision*/
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*unhide model*/
	//SetActorHiddenInGame(false);	

	
	/*register as primary weapon if warranted*/
	if (this->Implements<UPrimaryWeaponInterface>() && InventoryItem->InvManager->ReturnPrimaryWeapon() == nullptr)
		InventoryItem->InvManager->SetPrimaryWeapon(this);
	if (this->Implements<USecondaryWeaponInterface>() && InventoryItem->InvManager->ReturnSecondaryWeapon() == nullptr)
		InventoryItem->InvManager->SetSecondaryWeapon(this);
}

void AWeapon::OnEquippedFinished()
{
	AFirstifyCharacter* OwningPawn = Cast<AFirstifyCharacter>(GetOwner());
//	GetWorldTimerManager().ClearTimer(EquipHandler);
	
	if (!OwningPawn)
		return;

	/*update state*/
	SetState(EWeaponState::Equipped);

	//notify owner/user we've finished
	OwningPawn->OnEquippedFinished(this);
}

void AWeapon::MulticastOnEquipped_Implementation(AActor* User)
{
	SetOwner(User);

	//@TODO : consolidate call
	MultiAttachWeaponToCharacter();
	
	/*disable collision*/
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	/*enable visiblity*/
	//Mesh1P->SetVisibility(true, true);
	//Mesh3P->SetVisibility(true, true);	
}

void AWeapon::ClientOnEquipped_Implementation(AActor* User)
{
	//UE_LOG(LogFirstify, Log, TEXT("%s%s::ClientOnEquipped(%s)"), NETMODE_WORLD, *GetName(), *GetNameSafe(User));

	SetOwner(User);
	AttachToFirstPersonArms();
	InitFirstPersonArms();	
}

void AWeapon::OnUnequipped(AActor* User)
{	
	/*prevent us from trying to unequipping while already in the middle of one of these actions*/
	if (WeaponState == EWeaponState::Equipping || WeaponState == EWeaponState::UnEquipping)
		return;

	/*set state*/
	SetState(EWeaponState::UnEquipping);
}

/*OnUnEquippedFinished() - Used to clear out references and de-activate this weapon*/

void AWeapon::OnUnEquippedFinished()
{
	UE_LOG(LogFirstify, Log, TEXT("%s%s::OnUnEquippedFinished()"),NETMODE_WORLD, *GetName());

	/*initiate references*/
	AFirstifyCharacter* OwningPawn = Cast<AFirstifyCharacter>(GetOwner());

	/*set state*/
	SetState(EWeaponState::UnEquipped);
	//OwningPawn->FirstPersonArms->ClearAnimScriptInstance();

	if (OwningPawn)
	{
		
		//hide 1st person view && reset arms
		if (OwningPawn->IsLocallyControlled())
		{
			//OwningPawn->FirstPersonArms->SetRelativeLocation(OwningPawn->ArmsDefaultCameraOffset);
			//Mesh1P->SetVisibility(false, true);
		}

		/*notify owner/user that we've finished*/
		OwningPawn->OnUnEquippedFinished(this);
	}
}

bool AWeapon::IsPendingEquip()
{
	//if (GetWorldTimerManager().IsTimerActive(EquipHandler) || GetWorldTimerManager().IsTimerActive(UnEquipHandler))
	//	return true;
	if (WeaponState == EWeaponState::Equipping || WeaponState == EWeaponState::UnEquipping)
		return true;
	else
		return false;
}

void AWeapon::OnPickup(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s%s::OnPickup()"), NETMODE_WORLD,*GetName());

	if (GetOwner() != User)
		SetOwner(User);

	if (User->IsA<AFirstifyCharacter>())
	{
		UE_LOG(LogFirstify, Log, TEXT("%s::Valid Firstify Character - Attempting to Equip() weapon....."), *GetName());

		AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(User);
		Char->Equip(this);		
	}
}

bool AWeapon::CanPickup_Implementation(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::CanPickup_Implementation(%s)"), *GetName(), *GetNameSafe(User));

	return true;
}

void AWeapon::OnDropped(AActor*)
{
	UE_LOG(LogFirstifyInventory, Log, TEXT("%s::OnDropped"), *GetName());

	Mesh1P->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh3P->SetCollisionProfileName("Weapon");
	Mesh3P->SetAllBodiesSimulatePhysics(true);
}


//==================================================================
//==============================STATES==============================
//==================================================================

void AWeapon::SetState(EWeaponState NewState)
{
	PreviousState = WeaponState;
	WeaponState = NewState;
}

void AWeapon::OnRep_WeaponStateChanged()
{
	switch(WeaponState)
	{
		case EWeaponState::Equipping:
			OnEquipped(GetOwner());
	}
}



//======================================================================
//==============================ANIMATIONS==============================
//======================================================================

/*InitFirstPersonArms() - Initiates the First Person Arms w/ the Weapon's Blueprint Override
*
*
*
*/
void AWeapon::InitFirstPersonArms()
{	
	/*safety check*/
	if (!IsLocallyControlled())
		return;
	
	ReturnOwningPawn()->FirstPersonArms->SetArmAnimInstance(ArmsAnimBlueprint);
}

FVector AWeapon::ReturnArmsCameraOffset()
{
		return FirstPersonCameraOffset;
}

UAnimInstance* AWeapon::ReturnArmsAnimInstance()
{
	AFirstifyCharacter* OwningPawn = Cast<AFirstifyCharacter>(GetOwner());
	if (OwningPawn)
		return OwningPawn->FirstPersonArms->ArmsMesh->GetAnimInstance();;

	return nullptr;
}

void AWeapon::AttachToFirstPersonArms()
{
	/*safety check*/
	if (!ReturnOwningPawn())
		return;

	ReturnOwningPawn()->FirstPersonArms->AttachToRightHand(Mesh1P);
	InitFirstPersonArms();
}

void AWeapon::DetachFromFirstPersonArms()
{
	/*safety/owning client check*/
	if (!ReturnOwningPawn() || !ReturnOwningPawn()->IsLocallyControlled())
		return;

	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeapon::MultiAttachWeaponToCharacter_Implementation()
{
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

	if (GetOwner() && GetOwner()->IsA<AFirstifyCharacter>())
	{

		AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetOwner());
		AttachToComponent(Char->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
	}
}

//==================================
//=========ANIMATION EVENTS=========
//==================================

void AWeapon::OnAnimNotifyEquipFinished()
{
	OnEquippedFinished();
}

/*OnUnEquipAnimFinished() - Blueprint Callable event designed to be called from the Animaiton Blueprint
* Used to notify the weapon that the UnEquipping animation state has finished - and it is now safe to full UnEquip the weapon
*
*
*/
void AWeapon::OnAnimNotifyUnequipFinished()
{
	OnUnEquippedFinished();
}




//void AWeapon::PlayEquipAnimation()
//{
//	UAnimInstance* ArmsInstance = ReturnArmsAnimInstance();
//	AFirstifyCharacter* Pawn = Cast<AFirstifyCharacter>(GetOwner());
//
//	/*first person animations for local player only*/
//	if (Pawn && Pawn->IsLocallyControlled() && ArmsInstance)
//	{
//		if (ArmsEquipAnimation)
//			ArmsInstance->PlaySlotAnimationAsDynamicMontage(ArmsEquipAnimation, FName("DefaultSlot"), 0.0f, 0.25f);
//	}
//}
//
//void AWeapon::PlayUnEquipAnimation()
//{
//	UAnimInstance* ArmsInstance = ReturnArmsAnimInstance();
//	AFirstifyCharacter* Pawn = Cast<AFirstifyCharacter>(GetOwner());
//
//	/*first person animations for local player only*/
//	if (Pawn && Pawn->IsLocallyControlled() && ArmsInstance)
//	{
//		if (ArmsUnEquipAnimation)
//			ArmsInstance->PlaySlotAnimationAsDynamicMontage(ArmsUnEquipAnimation, FName("DefaultSlot"), 0.0f, 0.25f);
//	}
//}



//===========================================
//=================UTILITIES=================
//===========================================

AFirstifyCharacter* AWeapon::ReturnOwningPawn()
{
	return Cast<AFirstifyCharacter>(GetOwner());
}

bool AWeapon::IsLocallyControlled()
{
	if (ReturnOwningPawn() && ReturnOwningPawn()->IsLocallyControlled())
		return true;
	else
		return false;
}
