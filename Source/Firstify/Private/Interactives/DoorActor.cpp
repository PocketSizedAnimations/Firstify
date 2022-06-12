// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactives/DoorActor.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/MaterialBillboardComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ADoorActor::ADoorActor(const FObjectInitializer& ObjectInitiaizer)
{
	bOpensBothDirections = false;
	bLocked = false;

	RootScene = ObjectInitiaizer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootSceneComp"));
	SetRootComponent(RootScene);
	EditorIcon = ObjectInitiaizer.CreateDefaultSubobject<UBillboardComponent>(this, TEXT("EditorIconComp"));
	if (EditorIcon)
	{
		EditorIcon->SetupAttachment(GetRootComponent());
		EditorIcon->SetRelativeLocation(FVector(20, 0, 30));
	}

	
	PrimaryActorTick.bCanEverTick = false; //no need for Tick in this actor
}

#if WITH_EDITOR
void ADoorActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	/*get the name of the property*/
	FName PropertyName = PropertyChangedEvent.Property != NULL ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ADoorActor, bLocked))
	{
		if (bLocked)
			OnDoorLocked();
		else
			OnDoorUnlocked();
	}
}
#endif

// Called when the game starts or when spawned
void ADoorActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADoorActor::OpenDoor(AActor* User, bool bReverseDirection)
{
	bOpen = true;
}

bool ADoorActor::IsOpen()
{
	return bOpen;
}

void ADoorActor::OnDoorLocked()
{
#if WITH_EDITOR
	EditorIcon->SetSprite(DoorIconLocked);
#endif
}

void ADoorActor::OnDoorUnlocked()
{
#if WITH_EDITOR
	EditorIcon->SetSprite(DoorIcon);
#endif
}
