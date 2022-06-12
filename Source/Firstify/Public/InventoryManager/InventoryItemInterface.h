#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "InventoryItemInterface.generated.h"

UINTERFACE(Blueprintable, MinimalAPI)
class UInventoryItemInterface : public UInterface
{
	GENERATED_BODY()
};


class IInventoryItemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
		virtual void OnEquipped(AActor* User);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory", meta = (DisplayName = "On Equipped"))
		void BP_OnEquipped(AActor* User);
	UFUNCTION()
		virtual void OnUnequipped(AActor* User);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory", meta = (DisplayName = "On Unequipped"))
		void BP_OnUnequipped(AActor* User);

	UFUNCTION()
		virtual void OnPickup(AActor* User);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory", meta = (DisplayName = "On Picked Up"))
		void BP_OnPickup(AActor* User);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactive")
		bool CanPickup(class AActor* User);
	
	UFUNCTION()
		virtual void OnQuickuse(AActor* User);
	UFUNCTION(BlueprintImplementableEvent, BLueprintCallable, Category = "Inventory", meta = (DisplayName = "On Quickuse"))
		void BP_OnQuickuse(AActor* User);

	UFUNCTION()
		virtual void OnDropped(AActor* User);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, category = "Inventory", meta = (DisplayName = "On Dropped"))
		void BP_OnDropped(AActor* User);
};

