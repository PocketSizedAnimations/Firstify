// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FootstepSounds.generated.h"

USTRUCT(BlueprintType)
struct FFootstepSurfaceSounds
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PhysicalProperties)
		TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physical Properties")
		class USoundCue* SoundCue;

};

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UFootstepSounds : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Footprint Sounds")
		TArray<FFootstepSurfaceSounds> Sounds;
	
};
