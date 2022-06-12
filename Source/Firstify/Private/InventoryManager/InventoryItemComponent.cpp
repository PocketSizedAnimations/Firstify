// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManager/InventoryItemComponent.h"
#include "InventoryManager/InventoryItemInterface.h"
#include "InventoryManager/InventoryManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Firstify.h"


/*network*/
#include "Net/UnrealNetwork.h"


void UInventoryItemComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItemComponent, bInStorage);
	DOREPLIFETIME_CONDITION(UInventoryItemComponent, CurrentStackCount, COND_InitialOrOwner);
	DOREPLIFETIME_CONDITION(UInventoryItemComponent, MaxStackCount, COND_InitialOrOwner);
}

UInventoryItemComponent::UInventoryItemComponent()
{
	bCanPickup = true;

	bInStorage = false;

	CurrentStackCount = 1;
	MaxStackCount = 1;
	bRemoveOnDepletion = true;

	bAllowQuickslotting = true;
	QuickslotGroup = EQuickslot::None;
	
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UInventoryItemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryItemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryItemComponent::RegisterItem(AActor* NewOwner)
{
	if (!NewOwner)
		return false;

	InvManager = nullptr; //clear out old references
	InvManager = Cast<UInventoryManagerComponent>(NewOwner->GetComponentByClass(UInventoryManagerComponent::StaticClass()));
	
	/*set our Actor's owner to the new owner*/
	GetOwner()->SetOwner(NewOwner);
	
	//if we failed to get the new manager - we failed to register the item
	if (!InvManager)
		return false;
	

	return true;
}

void UInventoryItemComponent::Equip(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::Equip(%s)"), *GetName(), *GetNameSafe(User));	

	RegisterItem(User);

	if (GetOwner() && GetOwner()->Implements<UInventoryItemInterface>())
	{
		Cast<IInventoryItemInterface>(GetOwner())->OnEquipped(User);
		Cast<IInventoryItemInterface>(GetOwner())->Execute_BP_OnEquipped(GetOwner(),User);
	}

	/*broadcast to everyone listening that we've been equipped*/
	if (OnItemEquipped.IsBound())
	{
		UE_LOG(LogFirstify, Log, TEXT("%s::OnEquipped.IsBound() - Broadcasting..."), *GetName());
		OnItemEquipped.Broadcast(User);
	}
}

void UInventoryItemComponent::Unequip(AActor* User)
{
	if (GetOwner() && GetOwner()->Implements<UInventoryItemInterface>())
	{
		/*inform our actual item class things are being unequipped*/
		Cast<IInventoryItemInterface>(GetOwner())->OnUnequipped(User);
		Cast<IInventoryItemInterface>(GetOwner())->Execute_BP_OnUnequipped(GetOwner(),User);
	}

	/*broadcast to everyone that is listening that we've been unequipped*/
	if (OnItemUnequipped.IsBound())
		OnItemUnequipped.Broadcast(User);
}

void UInventoryItemComponent::Pickup(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::Pickup(%s)"), *GetName(), *GetNameSafe(User));
	UE_LOG(LogFirstify, Log, TEXT("%s::Pickup(): Owner = %s"), *GetName(), *GetNameSafe(GetOwner()));
	
	RegisterItem(User);
	GetOwner()->AttachToActor(User, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if (CanPickup(User) && OnItemPickup.IsBound())
		OnItemPickup.Broadcast(User);
}

bool UInventoryItemComponent::CanPickup(AActor* User)
{
	bool bCanUseOwner = true;

	if (GetOwner() && GetOwner()->Implements<UInventoryItemInterface>())
	{
		bCanUseOwner = Cast<IInventoryItemInterface>(GetOwner())->Execute_CanPickup(GetOwner(),User);
	}
	
	if (bCanPickup && bCanUseOwner)
		return true;
	else
		return false;
}

void UInventoryItemComponent::QuickUse(AActor* User)
{
	if (GetOwner() && GetOwner()->Implements<UInventoryItemInterface>())
	{
		Cast<IInventoryItemInterface>(GetOwner())->OnQuickuse(User); //C++ hook
		Cast<IInventoryItemInterface>(GetOwner())->Execute_BP_OnQuickuse(GetOwner(), User); //Blueprint hook
	}
}

void UInventoryItemComponent::OnDrop(AActor* User)
{
	if (!GetOwner())
		return;

	GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	GetOwner()->DetachRootComponentFromParent(true);
	GetOwner()->SetActorEnableCollision(true);
	GetOwner()->SetOwner(nullptr);
	

	if (GetOwner()->Implements<UInventoryItemInterface>())
	{
		Cast<IInventoryItemInterface>(GetOwner())->OnDropped(User); //C++ hook
		Cast<IInventoryItemInterface>(GetOwner())->Execute_BP_OnDropped(GetOwner(), User); //Blueprint hook
	}

	if (OnDropped.IsBound())
		OnDropped.Broadcast(User);
}


//=============================================================
//==========================INVENTORY==========================
//=============================================================

void UInventoryItemComponent::OnAddedToInventory(UInventoryManagerComponent* NewManager)
{
	if (!NewManager)
		return;

	/*set variable - which will be replicated to clients to disable collision/hide mesh*/
	bInStorage = true;

	/*broadcast our event*/
	if (OnAddToInventory.IsBound())
		OnAddToInventory.Broadcast(NewManager);
}

void UInventoryItemComponent::SubtractFromStack(uint8 Amount)
{
	uint8 RemainingAmount = CurrentStackCount - Amount;

	/*remove entire item from inventory if we run out of items*/
	if (bRemoveOnDepletion && RemainingAmount == 0)
	{
		InvManager->RemoveItem(GetOwner());
	}
	else
	{
		CurrentStackCount -= Amount; //subtract
	}
}

void UInventoryItemComponent::OnRep_Stored()
{
	/*disable collision for any item that's in storage (prevents wonkiness for clients thinking capsule are colliding)*/
	if (bInStorage == true)
	{
		GetOwner()->SetActorEnableCollision(ECollisionEnabled::NoCollision);
		//GetOwner()->SetActorHiddenInGame(true);
	}
}

