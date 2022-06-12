// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeapon.h"
#include "Weapons/PrimaryWeaponInterface.h"
#include "PrimaryWeapon.generated.h"

/**
 * DUMMY CLASS - SIMPLY ADDS IPRIMARYWEAPONINTERFACE FOR BLUEPRINTS TO EXTEND OFF OF WHICH IS USED FOR FILTERING IN EDITOR OF "PRIMARY" WEAPONS
 * EXTEND OFF OF THIS CLASS IF YOU WANT YOUR WEAPON TO BE EXPLITICITLY DEFINED AS A PRIMARY WEAPON ONLY
 */
UCLASS()
class FIRSTIFY_API APrimaryWeapon : public AShooterWeapon, public IPrimaryWeaponInterface
{
	GENERATED_BODY()

public:
	APrimaryWeapon(const FObjectInitializer& ObjectInitializer);
};
