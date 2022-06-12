// Copyright : 2020, Micah Parker & Pocket Sized Animations. All rights reserved


#include "GameInfo/FirstifyGameState.h"

/*core*/
#include "Engine/World.h"

/*projectiles*/
#include "Weapons/ProjectileManager.h"

void AFirstifyGameState::BeginPlay()
{
	Super::BeginPlay();


	ProjectileManager = GetWorld()->SpawnActor<AProjectileManager>(AProjectileManager::StaticClass());
}
