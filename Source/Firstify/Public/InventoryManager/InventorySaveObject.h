// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "InventorySaveObject.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UInventorySaveObject : public USaveGame
{
	GENERATED_BODY()
public:
	/*file name*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Object")
		FString SaveName;
	/*user/save index*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Object")
		uint8 SaveIndex;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "Save Object", meta = (MustImplement = "InventoryItemInterface"))
		TArray<TSubclassOf<class AActor>> DefaultItems;

	//=============================================================================================
	//==========================================FUNCTIONS==========================================
	//=============================================================================================
public:
	UInventorySaveObject();


public:
	/*call this to save the file name to disk*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Saving")
		void SaveInventory(FString SaveFileName);
	
};
