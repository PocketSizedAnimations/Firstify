#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Weapons/ProjectileManager.h"
#include "ShooterWeaponInstigatorInterface.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UShooterWeaponInstigatorInterface : public UInterface
{
    GENERATED_BODY()
};

class IShooterWeaponInstigatorInterface
{
    GENERATED_BODY()

public:
    /** Add interface function declarations here */


    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterWeapon")
        FProjectileTrajectory ReturnAimingTrajectory();
};