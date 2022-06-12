// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

UCLASS(abstract, NotBlueprintable)
class FIRSTIFY_API ADoorActor : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Setup")
		bool bOpensBothDirections;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
		bool bLocked;
	UPROPERTY()
		bool bOpen;
	
	
	/*the root of this actor that everything should attach to*/
	UPROPERTY(VisibleAnywhere, Category = "Rendering")
		class USceneComponent* RootScene;
	
	/*editor icon*/
		class UBillboardComponent* EditorIcon;
	UPROPERTY(EditDefaultsOnly, Category = "Editor")
			UTexture2D* DoorIcon;
	UPROPERTY(EditDefaultsOnly,Category = "Editor")
		UTexture2D* DoorIconLocked;

	
public:	
	// Sets default values for this actor's properties
	ADoorActor(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	UFUNCTION(BlueprintCallable, Category = "Interactives")
		virtual void OpenDoor(AActor* User, bool bReverseDirection = false);
	UFUNCTION(BlueprintPure, Category = "Door State")
		bool IsOpen();

	UFUNCTION()
		virtual void OnDoorLocked();
	UFUNCTION()
		virtual void OnDoorUnlocked();
};
