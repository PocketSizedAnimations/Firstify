// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Components/FirstifyCameraComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UFirstifyCameraComponent::UFirstifyCameraComponent()
{
	bAllowFreeLook = true;
	FreelookResetSpeed = 0.75f;

	MaxPitchUp = 89;
	MaxPitchDown = 89;
	MaxRotate = 85;
}

void UFirstifyCameraComponent::Pitch(float Value, bool bFreeLook)
{
	//AddRelativeRotation(FRotator(Value, 0, 0));
	if (bFreeLook)
	{
		FreeLook.X += Value;
		FreeLook.X = FMath::Clamp(FreeLook.X, FMath::Abs(MaxPitchDown) * -1, FMath::Abs(MaxPitchUp));
	}
	else
	{
		PitchAmt += Value;
		PitchAmt = FMath::Clamp(PitchAmt, FMath::Abs(MaxPitchDown) * -1, FMath::Abs(MaxPitchUp));
	}

	UpdateCameraRotation();
}

void UFirstifyCameraComponent::Rotate(float Value, bool bFreeLook)
{
	if (bFreeLook && bAllowFreeLook)
	{
		FreeLook.Y += Value;
		FreeLook.Y = FMath::Clamp(FreeLook.Y, FMath::Abs(MaxRotate) * -1, FMath::Abs(MaxRotate));
	}

	UpdateCameraRotation();
}

void UFirstifyCameraComponent::UpdateCameraRotation()
{
	/*handle rotation*/
	FRotator newRotation = GetAttachmentRootActor()->GetActorRotation();
	newRotation.Yaw += FreeLook.Y;
	SetWorldRotation(newRotation);
	
	/*add in pitch*/
	float newPitch = FMath::Clamp((PitchAmt + FreeLook.X), FMath::Abs(MaxPitchDown) * -1, FMath::Abs(MaxPitchUp));
	AddRelativeRotation(FRotator(newPitch, 0, 0));
}

void UFirstifyCameraComponent::ResetFreelookRotation()
{	
	FreeLook.X = 0;
	FreeLook.Y = 0;
	SetRelativeRotation(FRotator(0));
}

void UFirstifyCameraComponent::FreelookResetLerp()
{

}
