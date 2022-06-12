// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Gadgets/Gadget.h"
#include "InventoryManager/InventoryItemComponent.h"


/*network*/
#include "Net/UnrealNetwork.h"

// Sets default values
AGadget::AGadget(const FObjectInitializer& ObjectInitializer)
{
	
	/*create inventory component*/
	InventoryItemComponent = ObjectInitializer.CreateDefaultSubobject<UInventoryItemComponent>(this, TEXT("InventoryItemComp"));
	
	if (InventoryItemComponent)
	{
		InventoryItemComponent->MaxStackCount = 5;
	}
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AGadget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGadget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//=====================================
//=================USE=================
//=====================================


void AGadget::OnEquipped(AActor* User)
{

}


void AGadget::OnUnequipped(AActor* User)
{

}
//=======================================
//===============INVENTORY===============
//=======================================
bool AGadget::CanPickup_Implementation(AActor* User)
{
	return true;
}

void AGadget::OnQuickuse(AActor* User)
{

}


void AGadget::DecrementGadgetCount()
{
	InventoryItemComponent->SubtractFromStack(1);
}

void AGadget::RemoveGadgetFromInventory()
{

}

void AGadget::Reset()
{
	Destroy();

	Super::Reset();
}


