// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManager/InventoryManagerComponent.h"
#include "InventoryItemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEquipped, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnequipped, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickup, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddToInventory, UInventoryManagerComponent*, InventoryManager);



UCLASS( ClassGroup=(Inventory),HideCategories=("Collision","Cooking","Activation", "Sockets","Component Replication","Asset User Data"), meta=(BlueprintSpawnableComponent))
class FIRSTIFY_API UInventoryItemComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	/*registration*/
	UPROPERTY()
		class UInventoryManagerComponent* InvManager;
	
	/*state*/

	/*indicates whether or not this item is currently being stored*/
	UPROPERTY(Replicated, ReplicatedUsing = "OnRep_Stored")
		bool bInStorage;

	/*interaction*/
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Interaction")
		bool bCanPickup;

	/*stacking*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (DisplayName="Stack Count"))
		uint8 CurrentStackCount;
	/*how many of these can stack together*/
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
		uint8 MaxStackCount;
	/*should we remove this item from the inventory manager when stack count hits 0*/
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		bool bRemoveOnDepletion;


	/*item equipping*/
public:
	/*called whenever a use picks this item up*/
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnItemPickup OnItemPickup;
	/*called whenever this item is equipped*/
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnItemEquipped OnItemEquipped;
	/*called whenever this item is unequipped*/
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnItemUnequipped OnItemUnequipped;
	/*calld on the first time the item is added to a player's inventory*/
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnAddToInventory OnAddToInventory;
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnItemDropped OnDropped;
	/*quickslots*/
	UPROPERTY()
		bool bAllowQuickslotting;
	/*which Quickslot this will be assigned to - 0 is unassignable*/
	UPROPERTY(EditDefaultsOnly, Category = "Quickslots", meta=(EditCondition=bAllowQuickslotting))
		EQuickslot QuickslotGroup;

	/*ui*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		FText DisplayName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		class UTexture2D* PreviewImage;

	//=================================================================================================================
	//====================================================FUNCTIONS====================================================
	//=================================================================================================================
public:	
	// Sets default values for this component's properties
	UInventoryItemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*registration*/
protected:
	UFUNCTION()
		bool RegisterItem(AActor* NewOwner);

	//******************************************************************************************************************************************/
	/****the following functions automatically call their parent's corresponding functions - as long as it implements InventoryItemInterface****/
	/**************this is so that the component itself stays clean - but can still essentially be attached to anything*************************/
	//******************************************************************************************************************************************/
public:
	/*called whenever some one wants to Equip this item*/
	UFUNCTION()
		virtual void Equip(AActor* User);
	UFUNCTION()
		virtual void Unequip(AActor* User);
	UFUNCTION()
		virtual void Pickup(AActor* User);
	UFUNCTION()
		virtual bool CanPickup(AActor* User);

	UFUNCTION()
		virtual void QuickUse(AActor* User);
	
	UFUNCTION()
		virtual void OnDrop(AActor* User);

	//=============================================================
	//==========================INVENTORY==========================
	//=============================================================
public:
	/*called whenever this item gets added to an inventory manager*/
	UFUNCTION()
		virtual void OnAddedToInventory(UInventoryManagerComponent* NewManager);

	UFUNCTION()
		virtual void SubtractFromStack(uint8 Amount);

	UFUNCTION()
		virtual void OnRep_Stored();
};
