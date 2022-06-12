// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactives/DoorActor.h"
#include "DoubleDoor.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class FIRSTIFY_API ADoubleDoor : public ADoorActor
{
	GENERATED_BODY()
public:
	
	UPROPERTY(VisibleAnywhere, Category = "Rendering")
		class USceneComponent* LeftDoor;
	UPROPERTY(VisibleAnywhere, Category = "Rendering")
		class USceneComponent* RightDoor;

	UPROPERTY(VisibleAnywhere, Category = "Door")
		float LeftDoorSwing;
	UPROPERTY(VisibleAnywhere, Category = "Door")
		float RightDoorSwing;


	//=========================================================================
	//================================FUNCTIONS================================
	//=========================================================================
	ADoubleDoor(const FObjectInitializer& ObjectInitializer);


	virtual void OpenDoor(AActor* User, bool bReverseDirection) override;
};
