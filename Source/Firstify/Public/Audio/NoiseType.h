// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NoiseType.generated.h"


UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	Unknown,
	Footstep,
	ObjectFalling,
	Gunshot,
	Scream
};

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UNoiseType : public UObject
{
	GENERATED_BODY()
public:


	
};
