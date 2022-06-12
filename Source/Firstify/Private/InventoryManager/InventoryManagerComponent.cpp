// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManager/InventoryManagerComponent.h"
#include "InventoryManager/InventoryItemComponent.h"
#include "Players/FirstifyCharacter.h"

/*inventory items*/
#include "Weapons/Weapon.h"
#include "Weapons/ShooterWeapon.h"
#include "Weapons/Magazine.h"
#include "Weapons/ProjectileAsset.h"
#include "Weapons/PrimaryWeaponInterface.h"
#include "Weapons/SecondaryWeaponInterface.h"

/*logging*/
#include "Firstify.h"

/*utilities*/
#include "InventoryManager/InventorySaveObject.h"

/*network*/
#include "Net/UnrealNetwork.h"

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Ammo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, PrimaryWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, AlternativeWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, SecondaryWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot1, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot2, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot3, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot4, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot5, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot6, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot7, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot8, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Quickslot9, COND_OwnerOnly);
}

// Sets default values for this component's properties
UInventoryManagerComponent::UInventoryManagerComponent()
{
	bAutoEquipWeapons = true;
	
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//SetIsReplicated(true);
	SetIsReplicatedByDefault(true);

	// ...
}


// Called when the game starts
void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();	

	UE_LOG(LogFirstify, Log, TEXT("%s %s::BeginPlay()"), NETMODE_WORLD, *GetName());
	SpawnDefaultInventory();	
}


// Called every frame
void UInventoryManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UInventoryManagerComponent::SpawnDefaultInventory()
{
	/*only server can call*/
	if (GetNetMode() == NM_Client)
		return;

	UE_LOG(LogFirstify, Log, TEXT("%s %s::SpawnDefaultInventory_Implementation()"),NETMODE_WORLD, *GetName());

	SpawnInventory(DefaultItems);
}

void UInventoryManagerComponent::SpawnInventory(const TArray<TSubclassOf<class AActor>>& InventoryItems)
{
	/*only server can call - cancel*/
	if (GetNetMode() == NM_Client)
		return;

	UE_LOG(LogFirstify, Log, TEXT("%s %s::SpawnInventory()"), NETMODE_WORLD, *GetName());


	/*property initialization*/
	FTransform SpawnTransform = FTransform(GetOwner()->GetActorRotation(), GetOwner()->GetActorLocation(), FVector(1, 1, 1));
	FActorSpawnParameters SpawnParams = FActorSpawnParameters();
	SpawnParams.bNoFail = true;
	//SpawnParams.Instigator = GetOwner();
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFirstifyCharacter* Pawn = Cast<AFirstifyCharacter>(GetOwner());


	/*spawn the items*/
	for (uint8 i = 0; i < InventoryItems.Num(); i++)
	{
		AActor* NewItem = GetWorld()->SpawnActor<AActor>(InventoryItems[i], SpawnTransform, SpawnParams);
		UInventoryItemComponent* ItemComp = ReturnItemComponent(NewItem);

		/*the item has a valid InventoryItemComponent*/
		if (ItemComp != nullptr)
		{
			AddItem(NewItem); //store it in our inventory

			/*assign to a Quickslot*/
			if (ItemComp->QuickslotGroup != EQuickslot::None)
				AssignToQuickslot(NewItem);

			/*assign primary weapon quickslot*/
			if (NewItem->Implements<UPrimaryWeaponInterface>() && ReturnPrimaryWeapon() == nullptr)
				SetPrimaryWeapon(NewItem);
			/*assign secondary weapon quickslot*/
			if (NewItem->Implements<USecondaryWeaponInterface>() && ReturnSecondaryWeapon() == nullptr)
				SetSecondaryWeapon(NewItem);


			/*weapons*/
			if (AWeapon* Weap = Cast<AShooterWeapon>(NewItem))
			{
				/*shootey-type-weapons*/
				if (AShooterWeapon* ShooterWeap = Cast<AShooterWeapon>(Weap))
				{
					/*per magazine - advanced ammo tracking*/
					if (bTrackAmmoPerMagazine)
					{
					}
					/*standard ammo tracking*/
					else
					{
						AddAmmo(ShooterWeap->ReturnAmmoClass(), ShooterWeap->ReturnAmmoCountOnSpawn());
					}
				}


				/*auto-equip weapon*/
				if (bAutoEquipWeapons && Pawn && !Pawn->HasWeaponInHand())
				{
					//NewItem->SetActorHiddenInGame(false);
					Pawn->Equip(NewItem);
				}
			}
		}
	}
}

void UInventoryManagerComponent::LoadInventorySave(UInventorySaveObject* InventorySaveObject)
{
	/*server-only function*/
	if (GetNetMode() == NM_Client)
		return;

	if (!IsValid(InventorySaveObject))
		return;

	/*pass the item structure over to us - and begin loading inventory*/
	DefaultItems = InventorySaveObject->DefaultItems;
	SpawnDefaultInventory();
}


void UInventoryManagerComponent::AddItem(AActor* Item)
{
	/*only server can call*/
	if (GetNetMode() >= NM_Client)
		return;

	/*safety check*/
	if (!Item || !ReturnItemComponent(Item))
		return;
	
	/*register this item to us if needed*/
	if (!IsItemRegistered(Item))
		RegisterItem(Item);

	/*prevent us from adding the same instance*/
	if (ContainsItem(Item))
		return;
	

	/******************/
	/****begin add****/
	/******************/

	/*automatically turn off collision and hide it*/
	Item->SetActorEnableCollision(ECollisionEnabled::NoCollision);
	//Item->SetActorHiddenInGame(true);

	/*attach item to player for relevancy sake*/
	Item->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	/*add to list of inventory*/
	Items.Add(FInventoryItem(Item));

	/*notify item we added it*/
	if (ReturnItemComponent(Item))
		ReturnItemComponent(Item)->OnAddedToInventory(this);
}

bool UInventoryManagerComponent::ContainsItem(AActor* Item)
{
	if (!Item)
		return false;

	for (uint8 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].Item == Item)
			return true;
	}

	return false;
}

void UInventoryManagerComponent::RegisterItem(AActor* Item)
{
	/*only server can call*/
	if (GetNetMode() >= NM_Client)
		return;

	if (!Item || !ReturnItemComponent(Item))
		return;

	/*set us as their owner*/
	ReturnItemComponent(Item)->InvManager = this;
}

/*returns TRUE if this item is registered to this particular Inventory Manager*/
bool UInventoryManagerComponent::IsItemRegistered(AActor* Item)
{
	if (!Item || !ReturnItemComponent(Item))
		return false;

	if(ReturnItemComponent(Item)->InvManager == this)
		return true;
	else
		return false;
}


UInventoryItemComponent* UInventoryManagerComponent::ReturnItemComponent(AActor* Item)
{
	if (!Item)
		return nullptr;

	return Cast<UInventoryItemComponent>(Item->GetComponentByClass(UInventoryItemComponent::StaticClass()));
}

void UInventoryManagerComponent::RemoveItem(AActor* Item, bool bDestroy)
{
	if (!Item)
		return;

	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].Item == Item)
		{
			Items.RemoveAt(i);
			UnregisterItem(Item);
			RemoveFromQuickslot(Item);

			if (bDestroy)
				Item->Destroy();
		}
	}
}

void UInventoryManagerComponent::UnregisterItem(AActor* Item)
{
	/*servers only*/
	if (!Item || GetNetMode() == NM_Client)
		return;


	if (UInventoryItemComponent* ItemComp = ReturnItemComponent(Item))
	{
		ItemComp->InvManager = nullptr;
	}
}

//===============================================
//=====================WEAPONS===================
//===============================================

AWeapon* UInventoryManagerComponent::ReturnPrimaryWeapon()
{
	return PrimaryWeapon;
}

bool UInventoryManagerComponent::SetPrimaryWeapon(AActor* Item)
{
	if (Item && Item->IsA<AWeapon>() && Item->Implements<UPrimaryWeaponInterface>())
		PrimaryWeapon = Cast<AWeapon>(Item);

	if (PrimaryWeapon)
		return true;
	else
		return false;

}

AWeapon* UInventoryManagerComponent::ReturnSecondaryWeapon()
{
	return SecondaryWeapon;
}

bool UInventoryManagerComponent::SetSecondaryWeapon(AActor* Item)
{
	if (Item && Item->IsA<AWeapon>() && Item->Implements<USecondaryWeaponInterface>())
	{
		SecondaryWeapon = Cast<AWeapon>(Item);
	}

	if (SecondaryWeapon)
		return true;
	else
		return false;
}

EQuickslot UInventoryManagerComponent::AssignToQuickslot(AActor* Item)
{
	if (!Item)
		return EQuickslot::None;

	/*grab the item component*/
	if (UInventoryItemComponent* ItemComp = ReturnItemComponent(Item))
	{
		/*assign appropriate quickslot to item*/
		switch (ItemComp->QuickslotGroup)
		{
		case EQuickslot::Quickslot1:
			if (Quickslot1 == nullptr)
				Quickslot1 = Item;
			return EQuickslot::Quickslot1;
		case EQuickslot::Quickslot2:
			if (Quickslot2 == nullptr)
				Quickslot2 = Item;
			return EQuickslot::Quickslot2;
		case EQuickslot::Quickslot3:
			if (Quickslot3 == nullptr)
				Quickslot3 = Item;
			return EQuickslot::Quickslot3;
		case EQuickslot::Quickslot4:
			if (Quickslot4 == nullptr)
				Quickslot4 = Item;
			return EQuickslot::Quickslot4;
		case EQuickslot::Quickslot5:
			if (Quickslot5 == nullptr)
				Quickslot5 = Item;
			return EQuickslot::Quickslot5;
		case EQuickslot::Quickslot6:
			if (Quickslot6 == nullptr)
				Quickslot6 = Item;
			return EQuickslot::Quickslot6;
		case EQuickslot::Quickslot7:
			if (Quickslot7 == nullptr)
				Quickslot7 = Item;
			return EQuickslot::Quickslot7;
		case EQuickslot::Quickslot8:
			if (Quickslot8 == nullptr)
				Quickslot8 = Item;
			return EQuickslot::Quickslot8;
		case EQuickslot::Quickslot9:
			if (Quickslot9 == nullptr)
				Quickslot9 = Item;
			return EQuickslot::Quickslot9;
		default:
			return EQuickslot::None;
		}
	}

	return EQuickslot::None;
}

void UInventoryManagerComponent::RemoveFromQuickslot(AActor* Item)
{	
	if (UInventoryItemComponent* ItemComp = ReturnItemComponent(Item))
	{
		/*clear appropriate quickslot*/
		switch (ItemComp->QuickslotGroup)
		{
		case EQuickslot::Quickslot1:
			if (Quickslot1 == Item)
				ClearQuickslot(EQuickslot::Quickslot1);
			return;
		case EQuickslot::Quickslot2:
			if (Quickslot2 == Item)
				ClearQuickslot(EQuickslot::Quickslot2);
			return;
		case EQuickslot::Quickslot3:
			if (Quickslot3 == Item)
				ClearQuickslot(EQuickslot::Quickslot3);
			return;
		case EQuickslot::Quickslot4:
			if (Quickslot4 == Item)
				ClearQuickslot(EQuickslot::Quickslot4);
			return;
		case EQuickslot::Quickslot5:
			if (Quickslot5 == Item)
				ClearQuickslot(EQuickslot::Quickslot5);
			return;
		case EQuickslot::Quickslot6:
			if (Quickslot6 == Item)
				ClearQuickslot(EQuickslot::Quickslot6);
			return;
		case EQuickslot::Quickslot7:
			if (Quickslot7 == Item)
				ClearQuickslot(EQuickslot::Quickslot7);
			return;
		case EQuickslot::Quickslot8:
			if (Quickslot8 == Item)
				ClearQuickslot(EQuickslot::Quickslot8);
			return;
		case EQuickslot::Quickslot9:
			if (Quickslot9 == Item)
				ClearQuickslot(EQuickslot::Quickslot9);
			return;
		default:
			return;
		}
	}
}

void UInventoryManagerComponent::ClearQuickslot(EQuickslot Slot)
{
	/*clear appropriate quickslot*/
	switch (Slot)
	{
		case EQuickslot::Quickslot1:
			Quickslot1 = nullptr;
		case EQuickslot::Quickslot2:
			Quickslot2 = nullptr;
		case EQuickslot::Quickslot3:
			Quickslot3 = nullptr;
		case EQuickslot::Quickslot4:
			Quickslot4 = nullptr;
		case EQuickslot::Quickslot5:
			Quickslot5 = nullptr;
		case EQuickslot::Quickslot6:
			Quickslot6 = nullptr;
		case EQuickslot::Quickslot7:
			Quickslot7 = nullptr;
		case EQuickslot::Quickslot8:
			Quickslot8 = nullptr;
		case EQuickslot::Quickslot9:
			Quickslot9 = nullptr;
		default:
			return;
	}
}


	AActor* UInventoryManagerComponent::ReturnQuickslotItem(EQuickslot Slot)
{

	switch (Slot)
	{
		case EQuickslot::Quickslot1:
			return Quickslot1;
		case EQuickslot::Quickslot2:
			return Quickslot2;
		case EQuickslot::Quickslot3:
			return Quickslot3;
		case EQuickslot::Quickslot4:
			return Quickslot4;
		case EQuickslot::Quickslot5:
			return Quickslot5;
		case EQuickslot::Quickslot6:
			return Quickslot6;
		case EQuickslot::Quickslot7:
			return Quickslot7;
		case EQuickslot::Quickslot8:
			return Quickslot9;
		case EQuickslot::Quickslot9:
			return Quickslot9;
		default:
			return nullptr;
	}


	return nullptr;
}

bool UInventoryManagerComponent::HasAmmoOfType(UProjectileAsset* ProjectileClass)
{
	if (bTrackAmmoPerMagazine)
	{
		for (auto Magazine : Magazines)
		{
			if (ProjectileClass == Magazine->AmmoClass && Magazine->RoundsInMagazine > 0)
				return true;
		}
	}
	else
	{
		for (auto AmmoType : Ammo)
		{
			if (ProjectileClass == AmmoType.AmmoClass && AmmoType.Ammo > 0)
				return true;
		}
	}

	return false;
}

int32 UInventoryManagerComponent::ReturnAmmoOnHand(UProjectileAsset* ProjectileClass)
{
	int32 TotalAmount = 0;
	for (auto AmmoEntry : Ammo)
	{
		if (ProjectileClass == AmmoEntry.AmmoClass)
			TotalAmount += AmmoEntry.Ammo;
	}

	return TotalAmount;
}




void UInventoryManagerComponent::AddAmmo(UProjectileAsset* ProjectileClass, int32 DepositAmount)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::AddAmmo(%s,%i)"), *GetName(), *GetNameSafe(ProjectileClass), DepositAmount);

	/*add to existing entry if possible*/
	for (int32 i = 0; i < Ammo.Num(); i++)
	{
		if (Ammo[i].AmmoClass == ProjectileClass)
		{
			Ammo[i].Ammo += DepositAmount;
			return;
		}
	}

	/*add the new ammo type to the list if not*/
	Ammo.Add(FAmmoTracker(ProjectileClass, DepositAmount));
}

int32 UInventoryManagerComponent::ReturnSubtractedAmmo(UProjectileAsset* ProjectileClass, int32 RequestedAmount)
{
	//how much we're returning
	int32 ReturnAmmo = 0;

	for (int32 i = 0; i < Ammo.Num(); i++)
	{
		if (Ammo[i].AmmoClass == ProjectileClass)
		{
			ReturnAmmo = FMath::Clamp(Ammo[i].Ammo, 0, RequestedAmount);
			Ammo[i].Ammo -= ReturnAmmo;
		}
	}

	return ReturnAmmo;
}

void UInventoryManagerComponent::AddMagazine(AMagazine* Magazine)
{

}

AMagazine* UInventoryManagerComponent::ReturnNextMagazine(UProjectileAsset* ProjectileClass)
{
	return nullptr;
}

void UInventoryManagerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);


}

