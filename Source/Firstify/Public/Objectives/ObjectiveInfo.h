// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "ObjectiveInfo.generated.h"



UENUM()
enum class EObjectiveStatus : uint8
{
	Active,
	InActive,
	Completed,
	Failed
};

/**
 * 
 */
UCLASS(abstract, Blueprintable)
class FIRSTIFY_API AObjectiveInfo : public AInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Objetive")
		FName Name;
	UPROPERTY()
		FName ID; 
	UPROPERTY(EditAnywhere, Category = "Objetive")
		FText Description;
	UPROPERTY()
		EObjectiveStatus Status;
	

	//=============================================================
	//==========================FUNCTIONS==========================
	//=============================================================

	AObjectiveInfo();
};
