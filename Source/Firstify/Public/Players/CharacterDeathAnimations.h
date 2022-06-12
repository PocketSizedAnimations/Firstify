// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterDeathAnimations.generated.h"

USTRUCT(BlueprintType)
struct FDeathAnimationGroup
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Death Animation")
		FName AnimationGroup;
	UPROPERTY(EditAnywhere, Category = "Death Animation")
		float BlendInTime = 0.35f;
	UPROPERTY(EditAnywhere, Category = "Death Animation")
		TArray<class UAnimSequenceBase*> Animations;

};

/**
 * 
 */
UCLASS(Blueprintable)
class FIRSTIFY_API UCharacterDeathAnimations : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:


	UPROPERTY(EditDefaultsOnly, Category = "Animations")
		TArray<FDeathAnimationGroup> DeathAnimations = TArray<FDeathAnimationGroup>();

	
	
	//=====================================================================================
	//======================================FUNCTIONS======================================
	//=====================================================================================

	/*randomly selects a death animation from the passed in group*/
	UFUNCTION(BlueprintCallable, Category = "Animations")
		class UAnimSequenceBase* ReturnDeathAnimation(FName GroupID);

};
