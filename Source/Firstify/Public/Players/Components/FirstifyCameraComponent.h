// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "FirstifyCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UFirstifyCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		bool bAllowFreeLook;
	UPROPERTY(EditAnywhere, Category = "Camera Settings", meta = (UIMax = 89, ClampMax = 89))
		float MaxPitchUp;
	UPROPERTY(EditAnywhere, Category = "Camera Settings", meta = (UIMax = 89, ClampMax = 89))
		float MaxPitchDown;
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		float MaxRotate;
	
	UPROPERTY()
		float PitchAmt;
	UPROPERTY()
		FVector2D FreeLook;
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		float FreelookResetSpeed;
	FTimerHandle FreelookLerpHandler; //timer for resetting freelook

	//==================================================================
	//=============================FUNCTION=============================
	//==================================================================
	UFirstifyCameraComponent();

	UFUNCTION()
		virtual void Pitch(float Value, bool bFreeLook = false);
	UFUNCTION()
		virtual void Rotate(float Value, bool bFreeLook = false);
	UFUNCTION()
		virtual void UpdateCameraRotation();
	UFUNCTION()
		virtual void ResetFreelookRotation();
	//lerp function that smooth resets the camera back to a rotation of 0 for the client
	UFUNCTION()
		void FreelookResetLerp();
};
