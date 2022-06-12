// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/PrimaryWeapon.h"
#include "Weapons/ProjectileAssaultRifle.h"

APrimaryWeapon::APrimaryWeapon(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ProjectileAsset = NewObject<UProjectileAssaultRifle>();
}