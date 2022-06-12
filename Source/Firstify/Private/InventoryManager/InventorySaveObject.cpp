// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "InventoryManager/InventorySaveObject.h"

/*utilities*/
#include "Kismet/GameplayStatics.h"

UInventorySaveObject::UInventorySaveObject()
{
	SaveName = "";
	SaveIndex = 0;
}

void UInventorySaveObject::SaveInventory(FString SaveFileName)
{
	if (SaveName.IsEmpty() || SaveName == "")
	{
		return;
	}

	/*@TODO: FILE SAFETY*/
	UGameplayStatics::SaveGameToSlot(this, SaveName, SaveIndex);
}
