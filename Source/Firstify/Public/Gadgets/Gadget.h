// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryManager/InventoryItemInterface.h"
#include "Gadget.generated.h"

UCLASS(abstract, Blueprintable)
class FIRSTIFY_API AGadget : public AActor, public IInventoryItemInterface
{
	GENERATED_BODY()
public:


	/*components*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
		class UInventoryItemComponent* InventoryItemComponent;

//=============================================================================================
//==========================================FUNCTIONS==========================================
//=============================================================================================
	
public:	
	// Sets default values for this actor's properties
	AGadget(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//=====================================
	//=================USE=================
	//=====================================
public:



	//=======================================
	//===============INVENTORY===============
	//=======================================
	virtual void OnEquipped(AActor* User) override;
	
	virtual void OnUnequipped(AActor* User) override;

	virtual bool CanPickup_Implementation(AActor* User) override;

	virtual void OnQuickuse(AActor* User) override;

	//===============================================
//===================INVENTORY===================
//===============================================
protected:
	UFUNCTION()
		virtual void DecrementGadgetCount();
	UFUNCTION()
		virtual void RemoveGadgetFromInventory();


	//=====================================
	//=============LEVEL RESET=============
	//=====================================
	UFUNCTION()
		virtual void Reset() override;


};
