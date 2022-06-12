// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileAsset.h"
#include "NiagaraSystem.h"

/// <summary>
/// 
/// </summary>
UProjectileAsset::UProjectileAsset()
{
	Damage = 30.0f;
	BulletCount = 1;
	BulletVelocity = 922.0;
	TracerVelocity = BulletVelocity * 0.75f;
	EffectiveRange = 800.0;
	ExitWoundRange = 500;
	ImpactFXSize = EImpactFXSize::Medium;
	ImpactMomentum = 100000.0f;

	/*fx*/
	UNiagaraSystem* tracerFX = ConstructorHelpers::FObjectFinder<UNiagaraSystem>(TEXT("/Firstify/FX/Systems/Tracer")).Object;
	if (tracerFX)
		TracerFX = tracerFX;
}
