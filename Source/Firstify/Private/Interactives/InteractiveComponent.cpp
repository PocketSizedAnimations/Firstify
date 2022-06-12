// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactives/InteractiveComponent.h"
#include "GameFramework/Actor.h"
#include "Firstify.h"

// Sets default values for this component's properties
UInteractiveComponent::UInteractiveComponent()
{
	bIsInteractive = true; //default to always true
	
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractiveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInteractiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInteractiveComponent::StartInteraction(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::StartInteraction(%s)"),*GetName(), *GetNameSafe(User));

	if (OnInteractionStarts.IsBound())
	{
		UE_LOG(LogFirstify, Log, TEXT("%s::OnInteractionStarts.IsBound()"), *GetName());
		OnInteractionStarts.Broadcast(User);
	}
}

void UInteractiveComponent::StopInteraction(AActor* User)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::StopInteraction(%s)"),*GetName(), *GetNameSafe(User));
	
	if (OnInteractionStops.IsBound())
		OnInteractionStops.Broadcast(User);
}

bool UInteractiveComponent::IsInteractive_Implementation(AActor* User)
{
	return bIsInteractive;
}

