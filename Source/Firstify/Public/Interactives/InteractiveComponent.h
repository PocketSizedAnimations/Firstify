// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractiveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStarts, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStops, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*, User);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIRSTIFY_API UInteractiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	//whether or not this component/owning actor can be interacted with currently
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
		bool bIsInteractive;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FOnInteractionStarts OnInteractionStarts;
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FOnInteractionStops OnInteractionStops;
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FOnInteractionCompleted OnInteractionCompleted;


	//=========================================================================================================
	//================================================FUNCTIONS================================================
	//=========================================================================================================

public:	
	// Sets default values for this component's properties
	UInteractiveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
		void StartInteraction(AActor* User);
	UFUNCTION()
		void StopInteraction(AActor* User);
	UFUNCTION(BlueprintNativeEvent, Category = "Interactive")
		bool IsInteractive(AActor* User);
	
};
