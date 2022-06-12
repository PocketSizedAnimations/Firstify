// Fill out you	r copyright notice in the Description page of Project Settings.


#include "Players/FirstifyPlayerController.h"

/*components*/
#include "Animations/Components/FirstPersonArmsComponent.h"

/*debug*/
#include "Firstify.h"

/*players*/
#include "Players/FirstifyCharacter.h"
#include "Players/Components/FirstifyCharacterMovement.h"

/*projectiles*/
#include "Weapons/ProjectileManager.h"

/*world*/
#include "GameInfo/FirstifyGameState.h"
#include "Engine/World.h"

/*ui*/
#include "Blueprint/UserWidget.h"


AFirstifyPlayerController::AFirstifyPlayerController()
{
	bNotifyOfTargetHits = true;
	/*classes we want to be notified of when we hit*/
	HitNotifyClasses.Add(APawn::StaticClass());

	/*preferences*/
	bToggleCrouch = true;
}

void AFirstifyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/*init projectile manager*/
	if (!ProjectileManager)
		ProjectileManager = GetWorld()->SpawnActor<AProjectileManager>(AProjectileManager::StaticClass());
	else
		ProjectileManager->Reset();

	if(!IsLocalPlayerController())
		InitHUD(HUDClass);
}

void AFirstifyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(InPawn);

	/*update HUD*/
	if (IsLocalPlayerController() && Char && Char->ReturnHUDClass())
	{
		InitHUD(Char->ReturnHUDClass());
	}

	ClientOnPossess(InPawn);	
}

void AFirstifyPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	ClientOnUnPossess();
}


void AFirstifyPlayerController::ClientOnPossess_Implementation(APawn* InPawn)
{
	//InitHUD(InPawn); //Pass over the Pawn we're possessing as it's not always immedietaly set as Owner which InitHUD() relies on
	
	/*reset camera/audio fade back to normal*/
	PlayerCameraManager->bFadeAudio = false;
	PlayerCameraManager->StartCameraFade(1.0f, 1.0f, 0.001f,FLinearColor(0,0,0), true, false); 
	PlayerCameraManager->StopCameraFade();
	

	if (Cast<AFirstifyCharacter>(InPawn))
	{
		Cast<AFirstifyCharacter>(InPawn)->FirstPersonArms->SetVisibility(true);
	}
}

void AFirstifyPlayerController::ClientOnUnPossess_Implementation()
{
	ClearHUD();
}

//=======================================
//==================HUD==================
//=======================================

void AFirstifyPlayerController::InitHUD(TSubclassOf<UUserWidget> NewHUDClass, FName WidgetName)
{
	if (!IsLocalPlayerController() || !NewHUDClass)
		return;

	if (HUD)
		ClearHUD();

	HUD = CreateWidget<UUserWidget>(this, NewHUDClass);

	/*safety check*/
	if (HUD)
		HUD->AddToViewport();
}

void AFirstifyPlayerController::ClientInitHUD_Implementation(TSubclassOf<UUserWidget> NewHUDClass)
{
	InitHUD(NewHUDClass);
}

void AFirstifyPlayerController::ClearHUD()
{
	if (IsValid(HUD))
	{
		HUD->RemoveFromParent();
		HUD = nullptr;
	}
}


void AFirstifyPlayerController::OnNotifyOfTargetHit(AActor* HitActor, FHitResult HitResults)
{
	/*safety check*/
	if (!HitActor || !bNotifyOfTargetHits || HitNotifyClasses.Num() < 1)
		return;

	/*check to see if it's a valid class*/
	for (auto Class : HitNotifyClasses)
	{
		/*valid class*/
		if (HitActor->IsA(Class))
		{

			if (OnTargetHit.IsBound())
				OnTargetHit.Broadcast(HitActor, HitResults);
		}
	}
}

void AFirstifyPlayerController::OnNotifyOfPawnHit(AActor* HitPawn, AActor* HitInstigator)
{

	if (OnPawnTakeDamage.IsBound())
		OnPawnTakeDamage.Broadcast(HitPawn,HitInstigator);
}

//===========================================
//=================SPECTATOR=================
//===========================================

void AFirstifyPlayerController::SwitchToSpectator()
{
	if (GetNetMode() == NM_Client)
		ServerSwitchToSpectator();

	StartSpectatingOnly();
}


void AFirstifyPlayerController::ServerSwitchToSpectator_Implementation()
{
	SwitchToSpectator();
}

ASpectatorPawn* AFirstifyPlayerController::SpawnSpectatorPawn()
{
	/*override where the Spectator should Spawn*/
	FVector NewSpawnLocation = GetSpawnLocation();
	NewSpawnLocation.Z += 50; //offset upwards to avoid spawning in the floor
	SetSpawnLocation(NewSpawnLocation);
	
	/*continue on wiht life*/
	return Super::SpawnSpectatorPawn();
}


//============================================
//===================CHEATS===================
//============================================

void AFirstifyPlayerController::DebugProjectiles()
{
	if (ProjectileManager)
	{
		ProjectileManager->ToggleDebugTracing();
	}

}

void AFirstifyPlayerController::DebugCharacterMovement()
{
	if (AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetPawn()))
	{
		Char->ReturnCharacterMovement()->bShowDebug = !Char->ReturnCharacterMovement()->bShowDebug;
	}
}

void AFirstifyPlayerController::DebugInteractives()
{
	if (AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetPawn()))
	{
		Char->bDebugInteractives = !Char->bDebugInteractives;
	}
}

void AFirstifyPlayerController::Suicide()
{
	if (AFirstifyCharacter* Char = Cast<AFirstifyCharacter>(GetPawn()))
	{
		Char->Suicide(this);
	}
}
