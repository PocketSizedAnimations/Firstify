// Copyright 2020, Micah Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FirstifyGameState.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTIFY_API AFirstifyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:

	class AProjectileManager* ProjectileManager;

	//=========================================================================
	//================================FUNCTIONS================================
	//=========================================================================
	virtual void BeginPlay() override;
};
