// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"
#include "Weapons/ProjectileShotgun.h"

AShotgun::AShotgun(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ReloadType = EReloadType::Shell;
	RoundsPerMagazine = 8;
	RoundsInMagazine = 8;
	MinBulletSpread = 11.0f;

	ProjectileAsset = ObjectInitializer.CreateDefaultSubobject<UProjectileShotgun>(this, TEXT("Default Projectile"));	
}