// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactives/DoubleDoor.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"

ADoubleDoor::ADoubleDoor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	LeftDoor = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("LeftDoorComp"));
	if (LeftDoor)
	{
		LeftDoor->SetupAttachment(GetRootComponent());
		LeftDoor->SetRelativeLocation(FVector(0, 120, 0));
	}
	RightDoor = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RightDoorComp"));
	if (RightDoor)
	{
		RightDoor->SetupAttachment(GetRootComponent());
		RightDoor->SetRelativeLocation(FVector(0, -120, 0));
	}

	if (EditorIcon)
	{
		const FString iconTexturePath = "/Firstify/Editor/Icons/xF_EditorIcon_DoubleDoor";
		DoorIcon = ConstructorHelpers::FObjectFinder<UTexture2D>(*iconTexturePath).Object;
		EditorIcon->SetSprite(DoorIcon);

		const FString iconLockedTexturePath = "/Firstify/Editor/Icons/xF_EditorIcon_DoubleDoor_Locked";
		DoorIconLocked = ConstructorHelpers::FObjectFinder<UTexture2D>(*iconLockedTexturePath).Object;
	}
}

void ADoubleDoor::OpenDoor(AActor* User, bool bReverseDirection)
{
	Super::OpenDoor(User, bReverseDirection);
	
	if (bOpensBothDirections && bReverseDirection)
	{		
		LeftDoor->SetRelativeRotation(FRotator(0, -140, 0));
		RightDoor->SetRelativeRotation(FRotator(0, 140, 0));
	}
	else
	{
		LeftDoor->SetRelativeRotation(FRotator(0, 140, 0));
		RightDoor->SetRelativeRotation(FRotator(0, -140, 0));
	}
}
