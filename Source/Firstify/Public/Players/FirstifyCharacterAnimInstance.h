// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FirstifyCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UFirstifyCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:


	//===============================================================
	//===========================FUNCTIONS===========================
	//===============================================================
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnTakeDamage(EBodyPart BodyPart, ESide SideHit);
	
};
