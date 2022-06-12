// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FirstifyCharacterMovement.generated.h"

/**
 * 
 */
UCLASS() 
class FIRSTIFY_API UFirstifyCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Debugging")
        bool bShowDebug;
  

    /*movement capabilities*/
public:
    UPROPERTY(EditAnywhere, Category = "Nav Movement|Movement Capabilities")
        bool bCanProne;
    UPROPERTY(EditAnywhere, Category = "Nav Movement|Movement Capabilities")
        bool bCanSprint;
    UPROPERTY(EditAnywhere, Category = "Nav Movement|Movement Capabilities")
        bool bCanSideSprint;
    UPROPERTY(EditAnywhere, Category = "Nav Movement|Movement Capabilities")
        bool bCanSprintBackwards;
    UPROPERTY(EditAnywhere, Category = "Nav Movement|Movement Capabilities")
        bool bCanVault;
    UPROPERTY(EditAnywhere, Category = "Nav Movement|Movement Capabilities")
        bool bCanMantle;

    //*****************************/
    /***********falling************/
    //*****************************/
    /*how far we've fallen*/
    UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Jumping / Falling")
        float FallHeight;
    /*our World.Z height when we first entered Falling state*/
    UPROPERTY()
        float FallStartZLocation;

    UPROPERTY(EditAnywhere, Category = "Character Movement: Jumping / Falling")
        bool bCanTakeFallDamage;
    //height at which we begin to take fall damage when landing
    UPROPERTY(EditAnywhere, Category = "Character Movement: Jumping / Falling")
        float FallDamageHeightThreshold;
    UPROPERTY(EditAnywhere, Category = "Character Movement: Jumping / Falling", meta = (DisplayName = "Minimum Fall Damage"))
        float FallDamageMin;
    UPROPERTY(EditAnywhere, Category = "Character Movement: Jumping / Falling", meta = (DisplayName = "Maximum Fall Damage"))
        float FallDamageMax;
    UPROPERTY(EditAnywhere, Category = "Character Movement: Jumping / Falling")
        float TerminalVelocity;

    /*mantling*/
public:
    //character wants to mantle ASAP
    UPROPERTY()
        bool bWantsToMantle;
    //we're currently mantling
    UPROPERTY()
        bool bMantling;
protected:
    //how high we can reach up to pull ourselves up
    UPROPERTY(EditAnywhere, Category = "Character Movement: Mantling / Vaulting", meta=(MinUI=0,MinClamp=0))
        int32 MaxMantleHeight;
    //how far our we can mantle an object from us
    UPROPERTY(EditAnywhere, Category = "Character Movement: Mantling / Vaulting", meta=(MiniUI=0,MinClamp=0))
        float MaxMantleDistance;
    UPROPERTY()
        uint16 ObstacleHeight;
    UPROPERTY()
        FVector ObstacleBlockingLoc;

    //where we will need to mantle to
    UPROPERTY()
        FVector MantleToLocation;
    UPROPERTY()
        bool bCanMantleToStand = false;
    UPROPERTY()
        bool bCanMantleToCrouch = false;
    
    


    /*sprinting*/
protected:
    UPROPERTY(Replicated)
        bool bWantsToSprint;
public:
    UPROPERTY(EditAnywhere, Category = "Character Movement: Sprinting")
        float MaxSprintSpeed;
    UPROPERTY(EditAnywhere, Category = "Character Movement: Sprinting", meta=(DisplayName="Ease In Rate"))
        float SprintEaseInRate;
    UPROPERTY(EditAnywhere, Category = "Character Movement: Sprinting", meta = (DisplayName = "Ease Out Rate"))
        float SprintEaseOutRate;

    /*input*/
protected:
    //how much input push a player is attempting
    UPROPERTY(BlueprintReadOnly, Category = "Input")
        float MovementInputAmount;
    //the last direction (in rotation form) the player attempted to move the capsule
    UPROPERTY(BlueprintReadOnly, Category = "Input")
        FRotator LastMovementInputRotation;
    UPROPERTY(BlueprintReadOnly, Category = "Orientation")
        FRotator LastVelocityRotation;

    /*speed*/
    UPROPERTY(BlueprintReadOnly, Category = "Speed")
        bool bMoving;
    //UPROPERTY(BlueprintReadOnly, Category = "Speed")
    //    FVector Acceleration;
    UPROPERTY(BlueprintReadOnly, Category = "Speed")
        FVector PreviousVelocity;


    //===========================================================================================================
    //=================================================FUNCTIONS=================================================
    //===========================================================================================================
public:
    UFirstifyCharacterMovement();
    virtual void InitializeComponent() override;

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


    //==============================================================
    //===========================MOVEMENT===========================
    //==============================================================
    public:
        virtual float GetMaxSpeed() const override;


    //===================================================================
    //=======================MOVEMENT CAPABILITIES=======================
    //===================================================================
public:
    UFUNCTION()
        bool CanEverSprint() const { return bCanSprint; }
    UFUNCTION()
        bool CanEverMantle() const { return bCanMantle; }
    UFUNCTION()
        bool CanEverVault() const { return bCanVault; }



    //===================================================================
    //=============================SPRINTING=============================
    //===================================================================
public:
    UFUNCTION()
        bool IsSprinting() const;
    UFUNCTION()
        bool CanSprint() const;
    UFUNCTION()
        void BeginSprint();
    UFUNCTION()
        void EndSprint();
protected:
    UFUNCTION(Server, Reliable, WithValidation)
        void ServerNotifyBeginSprint();
    UFUNCTION(Server,Reliable,WithValidation)
        void ServerNotifyEndSprint();


    //=====================================================
    //========================INPUT========================
    //=====================================================
protected:
    //returns true if this capsule has player INPUT applied (attempted input from keyboard/mouse/controller, not actual movement)
    UFUNCTION(BlueprintPure, Category = "Input")
        bool HasMovementInput() { return MovementInputAmount > 0 ? true : false; }

    //====================================================
    //=================FALLING/JUMPING====================
    //====================================================
 public:
     virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;


    //====================================================
    //======================MANTLING======================
    //====================================================
 public:
     UFUNCTION()
         void SetWantsToMantle(bool bDesiresMantle);
     UFUNCTION(Server, Reliable, WithValidation)
         void ServerSetWantsToMantle(bool bDesiresMantle);
     
     UFUNCTION()
         void PerformMantleMove();

     UFUNCTION()
         bool CanMantle();
 protected:
     /**/
     UFUNCTION()
         void MantleCheck();
     /*returns true if there is an object we can't step up on blocking us*/
     UFUNCTION()
         bool HasObstacleInFront(FHitResult HitResults);
     UFUNCTION()
         bool CanMantleToStand(FVector Location);
     UFUNCTION()
        uint32 CalcObstacleHeight();
     /*traces using the capsule shape to see if we'll run into anything*/
     UFUNCTION()
         bool CapsuleTrace(FVector StartLoc, FVector TraceDir, float Distance, float TraceHalfHeight, float TraceRadius, FHitResult& OutHitResult, bool bDrawDebug = false, FColor NoHitColor = FColor::Green, FColor HitColor = FColor::Red);


     //============================================================
     //======================MOVEMENT UPDATES======================
     //============================================================
     virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

    


     //==========================================================
     //=======================NETWORK DATA=======================
     //==========================================================

     class FIRSTIFY_API FSavedMove_FirstifyCharacter : public FSavedMove_Character
     {
         typedef FSavedMove_Character Super;

         FSavedMove_FirstifyCharacter()
         {

         }
     };

};
