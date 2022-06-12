// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FirstifyPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitTargetNotification, AActor*, HitActor, FHitResult, HitResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPawnTakeDamage, AActor*, PawnHit, AActor*, HitInstigator);

/**
 * 
 */
UCLASS()
class FIRSTIFY_API AFirstifyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:


	/*projectile manager*/
public:
	UPROPERTY()
		class AProjectileManager* ProjectileManager; //class is used to track (server)/simulate (client) projectiles in the game

	/*hud*/
protected:
	UPROPERTY(EditDefaultsOnly,Category = "HUD")
		TSubclassOf<class UUserWidget> HUDClass;
	UPROPERTY()
		class UUserWidget* HUD;
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
		bool bNotifyOfTargetHits;
	UPROPERTY(EditDefaultsOnly, Category = "HUD", meta=(EditCondition=bNotifyOftargetHits))
		TArray<TSubclassOf<AActor>> HitNotifyClasses;
	UPROPERTY(BlueprintAssignable)
		FOnHitTargetNotification OnTargetHit;

	UPROPERTY(BlueprintAssignable)
		FOnPawnTakeDamage OnPawnTakeDamage;

	/*user preferences*/
public:
	UPROPERTY(BlueprintReadWrite, Category = "Preferences")
		bool bToggleCrouch;


	//=======================================================================================
	//=======================================FUNCTIONS=======================================
	//=======================================================================================
public:
	AFirstifyPlayerController();


	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	UFUNCTION(Client, Reliable)
		virtual void ClientOnPossess(APawn* InPawn);
	UFUNCTION(Client, Reliable)
		virtual void ClientOnUnPossess();

	//=======================================
	//==================HUD==================
	//=======================================
protected:
	UFUNCTION()
		virtual void InitHUD(TSubclassOf<class UUserWidget> NewHUDClass, FName WidgetName = "HUD");
public:

	UFUNCTION(Client, Reliable)
		virtual void ClientInitHUD(TSubclassOf<UUserWidget> NewHUDClass);
	UFUNCTION()
		virtual void ClearHUD();

	/*a broadcast that this player successfully hit something*/
	UFUNCTION()
		virtual void OnNotifyOfTargetHit(AActor* HitActor, FHitResult HitResults);

	UFUNCTION()
		virtual void OnNotifyOfPawnHit(AActor* HitPawn, AActor* HitInstigator);

	//===========================================
	//=================SPECTATOR=================
	//===========================================
public:
	UFUNCTION(BlueprintCallable, Category = "Spectating", exec)
		virtual void SwitchToSpectator();
	UFUNCTION(Server, Reliable)
		virtual void ServerSwitchToSpectator();

protected:
	/*spawn the player as a spectator*/
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;


	//============================================
	//===================CHEATS===================
	//============================================

protected:
	UFUNCTION(exec)
		virtual void DebugProjectiles();
	UFUNCTION(exec)
		virtual void DebugCharacterMovement();
	UFUNCTION(exec)
		virtual void DebugInteractives();
	UFUNCTION(exec)
		virtual void Suicide();
	
};
