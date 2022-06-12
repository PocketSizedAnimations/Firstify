// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "FirstPersonArmsComponent.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(Sockets,ComponentTick,Clothing,MasterPoseComponent,Physics,Collision,SkinWeights,Navigation,VirtualTexture,Cooking,Activation,SkeletalMesh,Optimization,MaterialParameters,HLOD))
class FIRSTIFY_API UFirstPersonArmsComponent : public USceneComponent
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, Category = "First Person")
		class USkeletalMeshComponent* ArmsMesh;
	UPROPERTY(EditDefaultsOnly, Category = "First Person")
		TSubclassOf<class UFirstPersonAnimInstance> DefaultAnimBlueprint;

	UPROPERTY()
		FVector InitialCameraOffset;

	//===============================================================================
	//===================================FUNCTIONS===================================
	//===============================================================================

	UFirstPersonArmsComponent(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeComponent() override;

	UFUNCTION()
		virtual void SetArmAnimInstance(TSubclassOf<UAnimInstance> NewAnimInstance);
	UFUNCTION()
		virtual void ResetArmAnimInstance();


	UFUNCTION()
		virtual void OffsetMesh(FVector Offset);
	UFUNCTION()
		virtual void ResetMeshOffset();


	//===========================
	//========ATTACHMENTS========
	//===========================
	UFUNCTION()
		virtual void AttachToLeftHand(USceneComponent* Component);
	UFUNCTION()
		virtual void AttachToRightHand(USceneComponent* Component);
	UFUNCTION()
		virtual void DetachFromArms();
	
	
};
