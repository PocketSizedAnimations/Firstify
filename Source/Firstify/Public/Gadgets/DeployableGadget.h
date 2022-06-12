// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Gadgets/Gadget.h"
#include "DeployableGadget.generated.h"

/**
 * 
 */
UCLASS(abstract, Blueprintable)
class FIRSTIFY_API ADeployableGadget : public AGadget
{
	GENERATED_BODY()
public:

    /*collision type used for determing if we can place the deployable gadget*/
    UPROPERTY(EditDefaultsOnly, Category = "Deployment")
        TEnumAsByte<ECollisionChannel> CollisionChannel;
    UPROPERTY(EditDefaultsOnly, Category = "Deployment")
        float UseCooldown;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deployment")
        float MaxDeployDistance;


//===============================================================================
//===================================FUNCTIONS===================================
//===============================================================================
    ADeployableGadget(const FObjectInitializer& ObjectInitializer);

    //=====================================================
    //=====================INTERACTION=====================
    //=====================================================
public:
    /*attempts to spawn a new gadget at the appropriate place*/
    UFUNCTION(BlueprintCallable, Category = "Gadgets|Deployment")
        virtual void Deploy();
    UFUNCTION(Server, Reliable, WithValidation)
        virtual void ServerRequestDeploy();

    /*called on the gadget as soon as it's spawned in the world - and placed (called for all players)*/
    UFUNCTION(NetMulticast, Reliable)
        virtual void MulticastOnDeployed(AActor* User, FHitResult HitResults);
        
    /*blueprint hook*/
    UFUNCTION(BlueprintImplementableEvent, Category = "Gadget", meta = (DisplayName="On Deployed"))
        void BP_OnDeployed(AActor* User, FHitResult HitResults);

    virtual void OnQuickuse(AActor* User) override;

//====================================================
//=====================DEPLOYMENT=====================
//====================================================
protected:
    UFUNCTION()
        virtual ADeployableGadget* SpawnGadget(FHitResult& OutHitResults);

};
