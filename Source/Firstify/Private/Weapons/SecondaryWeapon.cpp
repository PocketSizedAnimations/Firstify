// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/SecondaryWeapon.h"
#include "Weapons/ProjectilePistol.h"


ASecondaryWeapon::ASecondaryWeapon(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	RoundsInMagazine = 8;
	RoundsPerMagazine = 8;
	bUsesMagazines = false;

	//ProjectileAsset = 
}
