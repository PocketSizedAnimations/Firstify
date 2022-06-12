// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PainSounds.generated.h"

USTRUCT(BlueprintType)
struct FPainSound
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pain Sounds")
		TSubclassOf<class UDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pain Sounds")
		class USoundCue* Sound;
};

/**
 * 
 */
UCLASS()
class FIRSTIFY_API UPainSounds : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		TArray<FPainSound> PainSounds;
};
