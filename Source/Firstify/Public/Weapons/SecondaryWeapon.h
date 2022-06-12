// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeapon.h"
#include "Weapons/SecondaryWeaponInterface.h"
#include "SecondaryWeapon.generated.h"

/**
 * DUMMY CLASS - SIMPLY ADDS ISECONDARYWEAPONINTERFACE FOR BLUEPRINTS TO EXTEND OFF OF WHICH IS USED FOR FILTERING IN EDITOR OF "SECONDARY" WEAPONS
 * EXTEND OFF THIS CLASS IF YOU WANT YOUR WEAPON TO BE EXPLICTITLY DEFINED AS A SECONDARY WEAPON ONLY
 */
UCLASS()
class FIRSTIFY_API ASecondaryWeapon : public AShooterWeapon, public ISecondaryWeaponInterface
{
	GENERATED_BODY()	
public:



	ASecondaryWeapon(const FObjectInitializer& ObjectInitializer);
};
