// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagerComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item", meta = (MustImplement = "InventoryItemInterface"))
		AActor* Item;


	FInventoryItem():Item(nullptr) {
		 
	};
	FInventoryItem(AActor* inItem) : Item(inItem) {};

};

USTRUCT(BlueprintType)
struct FAmmoTracker
{
	GENERATED_BODY()
public:

	UPROPERTY()
		UProjectileAsset* AmmoClass;
	UPROPERTY()
		int32 Ammo;

	FAmmoTracker():AmmoClass(nullptr), Ammo(0) {};
	FAmmoTracker(UProjectileAsset* ammoClass, int32 ammo): AmmoClass(ammoClass), Ammo(ammo){};
};

UENUM(BlueprintType)
enum class EQuickslot : uint8
{
	None,
	Quickslot1,
	Quickslot2,
	Quickslot3,
	Quickslot4,
	Quickslot5,
	Quickslot6,
	Quickslot7,
	Quickslot8,
	Quickslot9
};


UCLASS( ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent) )
class FIRSTIFY_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:

protected:
	/*list of all items stored*/
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
		class AWeapon* PrimaryWeapon;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
		class AWeapon* AlternativeWeapon;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
		class AWeapon* SecondaryWeapon;	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
		TArray<FInventoryItem> Items;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot1;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot2;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot3;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot4;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot5;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot6;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot7;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot8;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Quickslots")
		AActor* Quickslot9;

protected:
	///*automatically equips weapons when first added/spawned (if none-currently equipped)*/
	UPROPERTY(EditAnywhere,Category = "Inventory", meta = (DisplayName = "Auto Equip"))
		bool bAutoEquipWeapons;



public:
	/*turns on advanced ammo tracking - keeping bullets on a per magazine basis*/
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
		bool bTrackAmmoPerMagazine;
protected:
	/*all magazines within our inventory - requires bTrackAmmoPerMagazine*/
	UPROPERTY()
		TArray<class AMagazine*> Magazines;
	UPROPERTY(Replicated)
		TArray<FAmmoTracker> Ammo;

	
	UPROPERTY(EditAnywhere, Category = "Items", meta = (MustImplement = "InventoryItemInterface"))
		TArray<TSubclassOf<class AActor>> DefaultItems;


	//=================================================================================================
	//============================================FUNCTIONS============================================
	//=================================================================================================

public:	
	// Sets default values for this component's properties
	UInventoryManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/*spawns the default inventory held by the component's configuration (DefaultItems)*/
	UFUNCTION()
		virtual void SpawnDefaultInventory();
	/*spawns a specific set of inventory items*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		virtual void SpawnInventory(const TArray<TSubclassOf<class AActor>>& InventoryItems);
	/*loads a saved set of inventory from a saved file*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
		virtual void LoadInventorySave(class UInventorySaveObject* InventorySaveObject);

	//=======================================
	//=============ITEM HANDLING=============
	//=======================================
	
	/*puts item into "storage" (removing it from the world)*/
	UFUNCTION(BlueprintCallable, Category = "Items")
		virtual void AddItem(AActor* Item);
	UFUNCTION(BlueprintCallable, Category = "Items")
		bool ContainsItem(AActor* Item);
	UFUNCTION()
		virtual void RegisterItem(AActor* Item);
	UFUNCTION()
		bool IsItemRegistered(AActor* Item);
	UFUNCTION()
		UInventoryItemComponent* ReturnItemComponent(AActor* Item);
	UFUNCTION(BlueprintCallable, Category = "Items")
		virtual void RemoveItem(AActor* Item, bool bDestroy = true);
	UFUNCTION()
		virtual void UnregisterItem(AActor* Item);

	//===============================================
	//=====================WEAPONS===================
	//===============================================

public:
	UFUNCTION(BlueprintPure, Category = "Inventory")
		AWeapon* ReturnPrimaryWeapon();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool SetPrimaryWeapon(AActor* Item);
	UFUNCTION(BlueprintPure, category = "Inventory")
		AWeapon* ReturnSecondaryWeapon();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool SetSecondaryWeapon(AActor* Item);

	//==================================================
	//====================QUICKSLOTS====================
	//==================================================
public:
	/*returns back the slot that was assigned*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Quickslots")
		EQuickslot AssignToQuickslot(AActor* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Quickslots")
		void RemoveFromQuickslot(AActor* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Quickslots")
		void ClearQuickslot(EQuickslot Slot);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Quickslots")
		AActor* ReturnQuickslotItem(EQuickslot Slot);
	


	//===============================================
	//=================AMMO HANDLING=================
	//===============================================
public:
	UFUNCTION()
		bool HasAmmoOfType(UProjectileAsset* ProjectileClass);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ammunition")
		int32 ReturnAmmoOnHand(UProjectileAsset* ProjectileClass);

	UFUNCTION()
		void AddAmmo(UProjectileAsset* ProjectileClass, int32 DepositAmount);
	/*grabs the max amount of ammo available from the request*/
	UFUNCTION()
		int32 ReturnSubtractedAmmo(UProjectileAsset* ProjectileClass, int32 RequestedAmount);
	UFUNCTION()
		void AddMagazine(class AMagazine* Magazine);
	/*grabs the next magazine in the list for this ammo type*/
	UFUNCTION()
		class AMagazine* ReturnNextMagazine(UProjectileAsset* ProjectileClass);
	



	//=================================================
	//===================DESTRUCTION===================
	//=================================================
protected:
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy);
		
};
