// Fill out your copyright notice in the Description page of Project Settings.

#include "Firstify/Public/Players/FirstifyCharacter.h"
#include "Firstify/Public/Players/FirstifyPlayerController.h"


/*animations*/
#include "Components/SkeletalMeshComponent.h"
#include "Players/CharacterDeathAnimations.h"
#include "Players/FirstifyCharacterAnimInstance.h"
#include "Animations/FirstPersonAnimInstance.h"

/*audio*/
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Players/DataAssets/FootstepSounds.h"
#include "Players/DataAssets/PainSounds.h"

/*camera*/
#include "Players/Components/FirstifyCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

/*components*/
#include "Animations/Components/FirstPersonArmsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Players/Components/FirstifyCharacterMovement.h"
#include "Components/InputComponent.h"

/*core*/
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

/*collision*/
#include "Components/CapsuleComponent.h"

/*damage*/
#include "Gameplay/FirstifyDamageType.h"
#include "Gameplay/DamageType_FallDamage.h"

/*debugging*/
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Firstify.h"

/*interactives*/
#include "Interactives/InteractiveComponent.h"

/*input*/
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"

/*inventory*/
#include "InventoryManager/InventoryManagerComponent.h"
#include "InventoryManager/InventoryItemComponent.h"

/*materials*/
#include "Materials/MaterialInterface.h"

/*physics*/
#include "PhysicalMaterials/PhysicalMaterial.h"

/*network*/
#include "Net/UnrealNetwork.h"

/*timing*/
#include "TimerManager.h"

/*weapons*/
#include "Weapons/Weapon.h"
#include "Weapons/ShooterWeapon.h"
#include "Weapons/MeleeWeapon.h"


#pragma region INITIALIZATION
void AFirstifyCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstifyCharacter, Health);
	DOREPLIFETIME(AFirstifyCharacter, HealthState);
	DOREPLIFETIME(AFirstifyCharacter, BodyCondition);

	DOREPLIFETIME(AFirstifyCharacter, InHand);

	DOREPLIFETIME_CONDITION(AFirstifyCharacter, PitchAmount, COND_SkipOwner);
}


AFirstifyCharacter::AFirstifyCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UFirstifyCharacterMovement>(ACharacter::CharacterMovementComponentName))
{ 	
	InteractiveTraceLength = 130;

	/*inventory*/
	InventoryManager = ObjectInitializer.CreateDefaultSubobject<UInventoryManagerComponent>(this, TEXT("InvManagerComp"));
	if (InventoryManager)
	{
		
	}

	/*weapons*/
	bReloadKeyHeld = false;


	/*health*/
	Health = 100.0f;
	HealthState = EHealthState::Healthy;
	bFadeCameraOnDeath = true;
	bAutoSpectatateOnDeath = true;

	/*movement*/
	bInitialCrouchKeystroke = false;

#pragma region Bone Identification
	/*bone to body identifications*/
	HeadBones.Add("b_Head");
	NeckBones.Add("b_Neck");

	TorsoBones.Add("b_UpperSpine");
	TorsoBones.Add("b_MidSpine");
	TorsoBones.Add("b_LeftClavicle");
	TorsoBones.Add("b_RightClavicle");

	AbdomenBones.Add("b_LowerSpine");

	PelvisBones.Add("b_Pelvis");

	LeftArmBones.Add("b_LeftUpperArm");
	LeftArmBones.Add("b_LeftLowerArm");
	LeftArmBones.Add("b_LeftHand");

	RightArmBones.Add("b_RightUpperArm");
	RightArmBones.Add("b_RightLowerArm");
	RightArmBones.Add("b_RightHand");

	LeftLegBones.Add("b_LeftThigh");
	LeftLegBones.Add("b_LeftCalf");
	LeftLegBones.Add("b_LeftFoot");

	RightLegBones.Add("b_RightThigh");
	RightLegBones.Add("b_RightCalf");
	RightLegBones.Add("b_RightFoot");
#pragma endregion
	
	/*cheats*/
	bGodMode = false;

	/*camera*/
	//FirstPersonArms = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("FirstPersonArms"));
	//if (FirstPersonArms)
	//{
	//	FirstPersonArms->SetupAttachment(GetRootComponent());
	//	FirstPersonArms->SetRelativeLocation(FVector(0, 0, 80));
	//}
	

	FirstPersonArms = ObjectInitializer.CreateDefaultSubobject<UFirstPersonArmsComponent>(this, TEXT("FirstPersonArmsComp"));
	if(FirstPersonArms)
	{
		FirstPersonArms->SetupAttachment(GetRootComponent());
	}

	if (GetMesh())
	{
		GetMesh()->bOwnerNoSee = true;
		GetMesh()->bCastHiddenShadow = true; //casts the full shadow for the controlling player even though the mesh is hidden
		GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
		GetMesh()->bReceivesDecals = false;

		//const FString MannequinMeshPath = "/Firstify/Characters/Mesh/xF_Mannequin_Male_A";
		//USkeletalMesh* _mannequinMesh = ConstructorHelpers::FObjectFinder<USkeletalMesh>(*MannequinMeshPath).Object;
		//if (_mannequinMesh)
		//	GetMesh()->SkeletalMesh = _mannequinMesh;
	}

	CameraComponent = ObjectInitializer.CreateDefaultSubobject<UFirstifyCameraComponent>(this, TEXT("CameraComp"));
	if (CameraComponent)
	{
		//CameraComponent->SetupAttachment(CameraSpringComponent);
		CameraComponent->SetupAttachment(FirstPersonArms);
		CameraComponent->SetRelativeLocation(FVector(0, 0, 0));
		CameraComponent->SetRelativeRotation(FRotator(0, 0, 0));
		CameraComponent->FieldOfView = 90;
	}

	/*sockets*/
	RightHandSocketName = FName("RightHandSocket");
	LeftHandSocketName = FName("LeftHandSocket");
	RightFootSocketName = FName("RightFootSocket");
	LeftFootSocketName = FName("LeftFootSocket");


	/*audio setup*/
	//const FString GenericFootstepsPath = "/Firstify/Characters/Audio/xF_Footstep_Generic.xF_Footstep_Generic";
	//USoundCue* _genericFootstepSound = ConstructorHelpers::FObjectFinder<USoundCue>(*GenericFootstepsPath).Object;

	//if (_genericFootstepSound)
	//	GenericFootstep = _genericFootstepSound;

	/*tick*/
	PrimaryActorTick.bCanEverTick = true;
}


void AFirstifyCharacter::BeginPlay()
{
	Super::BeginPlay();	
	
	
	LastHealthUpdate = Health;

	/*unhide first person arms*/
	if (FirstPersonArms)
	{		
		FirstPersonArms->SetVisibility(true);
	}

	/*initiate Audio Component*/
	if(!AudioComponent)
	{ 
		AudioComponent = NewObject<UAudioComponent>(this);
		AudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		AudioComponent->RegisterComponent();
	}

}

#pragma endregion

void AFirstifyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	check(PlayerInputComponent);

	InputComponent->BindAction("Use", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::Use);

	/*movement*/
	InputComponent->BindAxis("MoveForward", this, &AFirstifyCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AFirstifyCharacter::MoveRight);
	InputComponent->BindAxis("Pitch", this, &AFirstifyCharacter::Pitch);
	InputComponent->BindAxis("Rotate", this, &AFirstifyCharacter::Rotate);

	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::Sprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AFirstifyCharacter::StopSprinting);


	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::Jump);
	InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &AFirstifyCharacter::StopJumping);

	/*stances*/
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::OnCrouchPressed);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Released,this,  &AFirstifyCharacter::OnCrouchReleased);
	
	
	/*camera*/
	InputComponent->BindAction("FreeLook", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::EnableFreelook);
	InputComponent->BindAction("FreeLook", EInputEvent::IE_Released, this, &AFirstifyCharacter::DisableFreelook);

	/*weapons*/
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::BeginFire);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AFirstifyCharacter::EndFire);
	InputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::Reload);
	InputComponent->BindAction("Reload", EInputEvent::IE_Released, this, &AFirstifyCharacter::ReloadReleased);
	InputComponent->BindAction("ADS", EInputEvent::IE_Pressed, this, &AFirstifyCharacter::AimDownSights);
	InputComponent->BindAction("ADS", EInputEvent::IE_Released, this, &AFirstifyCharacter::ReleaseSights);

	/*inventory*/
	InputComponent->BindAction("EquipPrimary", EInputEvent::IE_Released, this, &AFirstifyCharacter::SwitchToPrimary);
	InputComponent->BindAction("EquipSecondary", EInputEvent::IE_Released, this, &AFirstifyCharacter::SwitchToSecondary);
	InputComponent->BindAction("DropInHand", EInputEvent::IE_Released, this, &AFirstifyCharacter::DropInHand);

	/*quickslots*/
	InputComponent->BindAction("Quickslot1", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot1);
	InputComponent->BindAction("Quickslot2", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot2);
	InputComponent->BindAction("Quickslot3", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot3);
	InputComponent->BindAction("Quickslot4", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot4);
	InputComponent->BindAction("Quickslot5", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot5);
	InputComponent->BindAction("Quickslot6", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot6);
	InputComponent->BindAction("Quickslot7", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot7);
	InputComponent->BindAction("Quickslot8", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot8);
	InputComponent->BindAction("Quickslot9", EInputEvent::IE_Released, this, &AFirstifyCharacter::Quickslot9);
}

#pragma region POSSESSION

void AFirstifyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	/*notify our controller*/
	if (AFirstifyPlayerController* PC = Cast<AFirstifyPlayerController>(NewController))
	{
		PC->ClientOnPossess(this);
		PC->ClientInitHUD(HUDClass);
	}

	if (ReturnCurrentWeapon())
		ReturnCurrentWeapon()->ClientOnEquipped(this);
}

#pragma endregion

/*called every frame*/
void AFirstifyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotationRate();

	/*local-player updates*/
	if(IsLocallyControlled()) //only the local player needs to do this constantly during a Tick()
		CalcInteractives();
}


#pragma region INVENTORY
void AFirstifyCharacter::BeginFire()
{
	/*fire weapon if we have one*/
	if (InHand)
	{
		if (InHand->IsA<AWeapon>() && CanFire())
		{
			Cast<AWeapon>(InHand)->BeginFire();
		}
	}
}

void AFirstifyCharacter::EndFire()
{
	if (InHand)
	{
		if (InHand->IsA<AWeapon>())
		{
			Cast<AWeapon>(InHand)->EndFire();
		}
	}
}

bool AFirstifyCharacter::CanFire()
{
	if (IsDeadOrIncapacitated())
		return false;
	
	if (ReturnCurrentWeapon() == nullptr)
		return false;
	
	return true;
}

bool AFirstifyCharacter::CanAimDownSights()
{
	/*dead*/
	if (IsDeadOrIncapacitated())
		return false;

	/*no weapon*/
	if (!HasWeaponInHand())
		return false;

	/*shooter-weapon based checks*/
	if (AShooterWeapon* Weapon = ReturnInHand<AShooterWeapon>())
	{
		/*we're in the middle of reloading*/
		if (Weapon->IsReloading())
			return false;
	}


	return true;
}

void AFirstifyCharacter::AimDownSights()
{
	if (!CanAimDownSights())
		return;


	bAimingDownSights = true;
	GetWorldTimerManager().SetTimer(ADSHandler, this, &AFirstifyCharacter::AimDownSightsLERP, 0.01f, true);
}

void AFirstifyCharacter::AimDownSightsLERP()
{
	if (!HasWeaponInHand())
	{	
		bAimingDownSights = false;
		return;
	}

	/*timing*/
	float ZoomTime;
	float TimePassed = GetWorldTimerManager().GetTimerElapsed(ADSHandler);
	float Alpha = 0; //current zoom rate
	/*FOV*/
	float CurrFOV = CameraComponent->FieldOfView;
	float DesiredFOV;
	/*weapon mesh*/
	FVector CurrentArmLoc = FirstPersonArms->GetRelativeLocation();
	FVector DesiredArmLoc;
	

	/*set Desired Field of View based on whether we're zooming in or out*/
	if (bAimingDownSights) //zooming in
	{
		DesiredFOV = Cast<AShooterWeapon>(ReturnCurrentWeapon())->ReturnDesiredFOV();
		ZoomTime = Cast<AShooterWeapon>(ReturnCurrentWeapon())->AimDownSightsTime;
		DesiredArmLoc = GetDefault<AFirstifyCharacter>(GetClass())->FirstPersonArms->GetRelativeTransform().GetLocation() + Cast<AShooterWeapon>(ReturnCurrentWeapon())->AimOffset; //Combine default position w/ offset from weapon
	}
	else //zooming out
	{
		DesiredFOV = GetDefault<AFirstifyCharacter>(GetClass())->CameraComponent->FieldOfView;
		ZoomTime = Cast<AShooterWeapon>(ReturnCurrentWeapon())->AimDownSightsTime * 0.3;
		DesiredArmLoc = GetDefault<AFirstifyCharacter>(GetClass())->FirstPersonArms->GetRelativeTransform().GetLocation();
	}

	/*calculate Alpha value*/
	if (TimePassed <= 0)
		Alpha = 0.1f;
	else
		Alpha = TimePassed / ZoomTime;
	/*safety clamp to keep in valid range*/
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	/*adjust weapon mesh*/
	//DO STUFF HERE
	FVector NewArmLoc = FMath::Lerp(CurrentArmLoc,DesiredArmLoc,Alpha);
	FirstPersonArms->SetRelativeLocation(NewArmLoc);

	/*adjust FOV*/
	float NewFOV = FMath::Lerp(CurrFOV, DesiredFOV, Alpha);
	CameraComponent->SetFieldOfView(NewFOV);


	if (Alpha >= 1.0f)
		GetWorldTimerManager().ClearTimer(ADSHandler);
}

void AFirstifyCharacter::ReleaseSights()
{
	bAimingDownSights = false;

	if (!HasWeaponInHand())
		return;

	GetWorldTimerManager().SetTimer(ADSHandler, this, &AFirstifyCharacter::AimDownSightsLERP, 0.01f, true);
}

#pragma endregion

//=======================================================
//=======================INVENTORY=======================
//=======================================================

#pragma region INTERACTION

void AFirstifyCharacter::Equip(AActor* Item)
{
	UE_LOG(LogFirstify, Log, TEXT("%s%s::Equip(%s)"), NETMODE_WORLD,*GetName(), *GetNameSafe(Item));
	
	/*safety check*/
	if (!Item)
		return;
	/*check if server is forcing a replication update - and if not prevent double equipping what we already have*/
	else if (bInHandRepChange == false && Item == ReturnInHand())
		return;

	/*unequip whatever we have first - then return back to this*/
	if (ReturnInHand() != nullptr && Item != ReturnInHand())
	{		
		PendingInHand = Item; //notify we want to equip something
		UnEquip(ReturnInHand()); //unequipp what we have first
		return; //cancel - will be recalled by UnEquipped if PendingInHand exists
	}

	/*network (server || local client)*/
	if (GetNetMode() < NM_Client || IsLocallyControlled())
	{
		InHand = Item;
		InHand->SetOwner(this); //item needs an owner so clients can interact with it appropriately

		/*if this was not server-enforced (and thus already known) change && we are the owning client - then notify the server we wish to change*/
		if (bInHandRepChange == false && GetNetMode() == NM_Client)
		{
			/*notify server we are equipping this*/
			ServerNotifyEquip(Item);
		}
	}

	/*notify the item we've equipped it*/
	if (Item->GetComponentByClass(UInventoryItemComponent::StaticClass()))
	{		
		UInventoryItemComponent* ItemComp = Cast<UInventoryItemComponent>(Item->GetComponentByClass(UInventoryItemComponent::StaticClass()));
		ItemComp->Equip(this);
	}

	/*reset server-enforced change*/
	if (bInHandRepChange)
		bInHandRepChange = false;
}

void AFirstifyCharacter::EquipPending()
{
	Equip(PendingInHand);
	PendingInHand = nullptr;
}

/*ServerNotifyEquip():Validate - Cheat Protection*/
bool AFirstifyCharacter::ServerNotifyEquip_Validate(AActor* Item)
{
	/*@TODO*/
	return true;
}


/*ServerNotifyEquip() - Client has equipped the item on their end - begin doing the same*/
void AFirstifyCharacter::ServerNotifyEquip_Implementation(AActor* Item)
{
	Equip(Item);
}


void AFirstifyCharacter::OnEquippedFinished(AActor* Item)
{
	UE_LOG(LogFirstify, Log, TEXT("%s%s::OnEquippedFinished()"),NETMODE_WORLD, *GetName());
}


void AFirstifyCharacter::UnEquip(AActor* Item)
{
	UE_LOG(LogFirstify, Log, TEXT("%s::OnUnEquip(%s)"), *GetName(), *GetNameSafe(Item));


	/*let item know we're unequipping it*/
	if (Item->GetComponentByClass(UInventoryItemComponent::StaticClass()))
	{		
		UInventoryItemComponent* ItemComp = Cast<UInventoryItemComponent>(Item->GetComponentByClass(UInventoryItemComponent::StaticClass()));
		ItemComp->Unequip(this);
	}

	/*network*/
	/*locally owned client*/
	if (IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		/*notify server*/
		ServerNotifyUnEquip(Item);
	}
}

bool AFirstifyCharacter::ServerNotifyUnEquip_Validate(AActor* Item)
{
	return true;
}

/*ServerNotifyUnEquip() - Notification from Client > Server that we've UnEquipped*/
void AFirstifyCharacter::ServerNotifyUnEquip_Implementation(AActor* Item)
{
	UnEquip(Item);
}

void AFirstifyCharacter::OnUnEquippedFinished(AActor* Item)
{
	UE_LOG(LogFirstify, Log, TEXT("%s%s::OnUnEquippedFinished(%s)"),NETMODE_WORLD, *GetName(), *GetNameSafe(Item));

	if(Item == InHand)
		ClearInHand();

	/*if we have a weapon that is waiting to be equipped - then begin equipping it*/
	if (PendingInHand != nullptr)
	{
		FTimerHandle Test;
		GetWorldTimerManager().SetTimer(Test, this, &AFirstifyCharacter::EquipPending, 0.075f, false);
		//PendingInHand = nullptr;
	}
}


void AFirstifyCharacter::OnRep_InHandChanged()
{
	bInHandRepChange = true; //server is updating client - push client to update
	Equip(InHand); //perform actual equipping logic

	///*owning clients*/
	//if (IsLocallyControlled())
	//{
	//	if (ReturnCurrentWeapon())
	//		ReturnCurrentWeapon()->ClientOnEquipped(this);
	//}
	///*non-owning clients*/
	//else
	//{
	//	if (ReturnCurrentWeapon())
	//		ReturnCurrentWeapon()->MulticastOnEquipped(this);
	//}
}

void AFirstifyCharacter::SwitchToPrimary()
{
	if (!InventoryManager || InventoryManager->ReturnPrimaryWeapon() == nullptr)
		return;
	
	if (ReturnInHand() == InventoryManager->ReturnPrimaryWeapon())
		UnEquip(ReturnInHand());

	else if (InventoryManager->ReturnPrimaryWeapon())
		Equip(InventoryManager->ReturnPrimaryWeapon());
}

void AFirstifyCharacter::SwitchToAlternative()
{

}

void AFirstifyCharacter::SwitchToSecondary()
{
	/*return if we have no secondary or it's already equipped*/
	if (!InventoryManager || InventoryManager->ReturnSecondaryWeapon() == nullptr)
		return;

	if (ReturnInHand() == InventoryManager->ReturnSecondaryWeapon())
		UnEquip(ReturnInHand());

	else if (InventoryManager->ReturnSecondaryWeapon())
		Equip(InventoryManager->ReturnSecondaryWeapon());
}


void AFirstifyCharacter::Reload()
{
	bReloadKeyHeld = true;

	if (!InHand)
		return;

	/*shooter weapons*/
	if (InHand->IsA(AShooterWeapon::StaticClass()))
		Cast<AShooterWeapon>(InHand)->Reload();
}

/*ReloadRelesed() - Triggered whenever a player lets go of the reload key*/
void AFirstifyCharacter::ReloadReleased()
{
	bReloadKeyHeld = false;

	if (!InHand)
		return;

	/*shooter weapons*/
	if (AShooterWeapon* ShooterWeapon = Cast<AShooterWeapon>(ReturnInHand()))
	{
		ShooterWeapon->OnReloadKeyReleased();
	}
}

UInventoryItemComponent* AFirstifyCharacter::ReturnItemComponent(AActor* Item)
{
	return Cast<UInventoryItemComponent>(Item->GetComponentByClass(UInventoryItemComponent::StaticClass()));
}

void AFirstifyCharacter::Quickslot1()
{
	QuickUse(EQuickslot::Quickslot1);
}

void AFirstifyCharacter::Quickslot2()
{
	QuickUse(EQuickslot::Quickslot2);
}

void AFirstifyCharacter::Quickslot3()
{
	QuickUse(EQuickslot::Quickslot3);
}

void AFirstifyCharacter::Quickslot4()
{
	QuickUse(EQuickslot::Quickslot4);
}

void AFirstifyCharacter::Quickslot5()
{
	QuickUse(EQuickslot::Quickslot5);
}

void AFirstifyCharacter::Quickslot6()
{
	QuickUse(EQuickslot::Quickslot6);
}

void AFirstifyCharacter::Quickslot7()
{
	QuickUse(EQuickslot::Quickslot7);
}

void AFirstifyCharacter::Quickslot8()
{
	QuickUse(EQuickslot::Quickslot8);
}

void AFirstifyCharacter::Quickslot9()
{
	QuickUse(EQuickslot::Quickslot9);
}

//=======================================
//================IN HAND================
//=======================================

void AFirstifyCharacter::ClearInHand()
{
	InHand = nullptr;
}

void AFirstifyCharacter::DropInHand()
{
	UE_LOG(LogFirstifyInventory, Log, TEXT("%s:DropInHand()"), *GetName());

	if (ReturnInHand() == nullptr)
		return;	

	/*notify item*/
	if (ReturnItemComponent(InHand))
		ReturnItemComponent(InHand)->OnDrop(this);

	/*clear reference*/
	ClearInHand();

	/*reset first person arms*/
	FirstPersonArms->ResetArmAnimInstance();
}


//=========================================
//=================WEAPONS=================
//=========================================

bool AFirstifyCharacter::HasWeaponInHand()
{
	if (ReturnInHand() && ReturnInHand()->IsA(AWeapon::StaticClass()))
		return true;

	return false;
}

AWeapon* AFirstifyCharacter::ReturnCurrentWeapon()
{
	return Cast<AWeapon>(ReturnInHand());
}

AShooterWeapon* AFirstifyCharacter::ReturnCurrentShooterWeapon()
{
	return Cast<AShooterWeapon>(ReturnInHand());
}

AMeleeWeapon* AFirstifyCharacter::ReturnCurrentMeleeWeapon()
{
	return Cast<AMeleeWeapon>(ReturnInHand());
}

bool AFirstifyCharacter::IsAimingDownSights()
{
	return bAimingDownSights;
}

void AFirstifyCharacter::ReturnFireTrajectory(FVector& OutLocation, FVector& OutDirection)
{
	OutLocation = FirstPersonArms->GetComponentLocation();
	OutDirection = FirstPersonArms->GetComponentRotation().Vector().GetSafeNormal();
}

void AFirstifyCharacter::DebugTest()
{
	if (ReturnCurrentWeapon())
		ReturnCurrentWeapon()->ClientOnEquipped(this);
}


void AFirstifyCharacter::CalcInteractives()
{
	//we only want the owning client to do this for performance - server will validate any attempt to actually use on request - and kick cheaters
	if (!IsLocallyControlled())
		return;

	
	FHitResult HitResults;
	FMinimalViewInfo ViewInfo;
	CameraComponent->GetCameraView(FApp::GetDeltaTime(), ViewInfo);
	
	FVector StartLoc = ViewInfo.Location;
	FVector EndLoc = StartLoc + (CameraComponent->GetForwardVector() * InteractiveTraceLength);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);


	if (bDebugInteractives)
		DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Orange, false, 0.1f);

	/*check for blocking hit via trace*/
	if (GetWorld()->LineTraceSingleByChannel(HitResults, StartLoc, EndLoc, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams) && IsInteractive(HitResults.GetActor()))
	{		
		CurrentInteractive = HitResults.GetActor();		
	}
	/*otherwise try a sweeping hit in a wider area*/
	else if (GetWorld()->SweepSingleByChannel(HitResults,StartLoc,EndLoc,CameraComponent->GetForwardVector().ToOrientationQuat(),ECollisionChannel::ECC_GameTraceChannel1,FCollisionShape::MakeSphere(4.0f),CollisionParams) && IsInteractive(HitResults.GetActor()))
	{
		CurrentInteractive = HitResults.GetActor();
	}
	else
	{
		//clear the interactive as we're no longer looking at it
		if (CurrentInteractive)
			CurrentInteractive = nullptr;
	}

	if (bDebugInteractives)
	{
		FString DebugInteractive = "Interactive Object : " + GetNameSafe(CurrentInteractive);
		GEngine->AddOnScreenDebugMessage(-1, 0.01, FColor::Orange, DebugInteractive);

		if (CurrentInteractive)
			DrawDebugSphere(GetWorld(), CurrentInteractive->GetActorLocation(), 10, 24, FColor::Orange, false, 0.1f);
		else 
			DrawDebugSphere(GetWorld(), HitResults.Location, 4, 24, FColor::Red, false, 0.1f);
	}
}

bool AFirstifyCharacter::IsInteractive(AActor* InteractiveObject)
{
	/*validation check*/
	if (!InteractiveObject)
		return false;

	/*interactive component check*/
	UInteractiveComponent* InteractiveComp = Cast<UInteractiveComponent>(InteractiveObject->GetComponentByClass(UInteractiveComponent::StaticClass()));
	if(InteractiveComp && InteractiveComp->IsInteractive(this))
		return true;

	/*inventory item check*/
	UInventoryItemComponent* ItemComp = Cast<UInventoryItemComponent>(InteractiveObject->GetComponentByClass(UInventoryItemComponent::StaticClass()));
	if (ItemComp && ItemComp->CanPickup(this))
		return true;

	
	return false;
}


void AFirstifyCharacter::Use()
{	
	if (!CurrentInteractive)
		return;
		

	/*inventory items*/
	if (ReturnItemComponent(CurrentInteractive))
		PickupItem(CurrentInteractive);

	/*interactive object*/
	else if (UInteractiveComponent* InteractiveComp = Cast<UInteractiveComponent>(CurrentInteractive->GetComponentByClass(UInteractiveComponent::StaticClass())))
	{		
		InteractiveComp->StartInteraction(this);
		return;
	}	
}

bool AFirstifyCharacter::ServerRequestUse_Validate(AActor* InteractiveObject)
{
	return true;
}

void AFirstifyCharacter::ServerRequestUse_Implementation(AActor* InteractiveObject)
{
	CurrentInteractive = InteractiveObject;
	Use();
}

void AFirstifyCharacter::PickupItem(AActor* InteractiveActor)
{
	/*safety check*/
	if (!InteractiveActor)
		return;

	UInventoryItemComponent* ItemComp = ReturnItemComponent(InteractiveActor);

	/*perform pickup*/
	if (ItemComp && ItemComp->CanPickup(this))
	{
		if (GetNetMode() < NM_Client)
			ItemComp->Pickup(this);
		else
			ServerRequestUse(InteractiveActor);

		return;
	}
}

void AFirstifyCharacter::ReleaseGrabbedItem()
{

}

#pragma endregion


#pragma region MOVEMENT

UFirstifyCharacterMovement* AFirstifyCharacter::ReturnCharacterMovement()
{
	return Cast<UFirstifyCharacterMovement>(GetCharacterMovement());
}

void AFirstifyCharacter::Pitch(float Value)
{
	/*prevent the dead from moving*/
	if (IsDead() || HealthState == EHealthState::Incapacitated)
		return;

	if (Value == 0.0f)
		return;

	if (bFreelookEnabled)
		CameraComponent->Pitch(Value, true);
	else
	{		
		float InputChange = Value;
		float LastValue = FirstPersonArms->GetRelativeTransform().GetRotation().Rotator().Pitch;
		float NewValue = LastValue + InputChange;

		if ((Value < 0 && NewValue >= (FMath::Abs(CameraComponent->MaxPitchDown) * -1)) || Value > 0 && NewValue <= FMath::Abs(CameraComponent->MaxPitchUp))
			FirstPersonArms->AddLocalRotation(FRotator(InputChange, 0, 0));

		if (IsLocallyControlled() && GetNetMode() == NM_Client)
			ServerUpdatePitch(FirstPersonArms->GetComponentRotation().Pitch);

		//UpdateRotationRate();

		/*LastPitchAmount = PitchAmount;
		PitchAmount = FirstPersonArms->GetComponentRotation().Pitch;
		PitchRate = PitchAmount - LastPitchAmount;*/
	}
}

bool AFirstifyCharacter::ServerUpdatePitch_Validate(float NewPitch)
{
	return true;
}

void AFirstifyCharacter::ServerUpdatePitch_Implementation(float NewPitch)
{
	FRotator NewRotation = FirstPersonArms->GetComponentRotation();
	NewRotation.Pitch = NewPitch;

	FirstPersonArms->SetWorldRotation(NewRotation);
	PitchAmount = NewPitch;
}


/*OnRep_PitchAmount() - Updates remote client's pitch so clients can sync their firing location*/
void AFirstifyCharacter::OnRep_PitchAmount()
{
	FRotator NewRotation = FirstPersonArms->GetComponentRotation();
	NewRotation.Pitch = PitchAmount;

	//@TODO: Probably remove this altogether - since the component was switched all this rotation is replicated automatically
	//FirstPersonArms->SetWorldRotation(NewRotation); <---defunct!
}

void AFirstifyCharacter::Rotate(float Value)
{
	/*prevent the dead from moving - ooOOOOOooOooOO*/
	if (IsDead() || HealthState == EHealthState::Incapacitated)
		return;

	/*track direction for animations*/
	TurnDirection = ETurnDirection::None;

	if (Value > 0.0f)
		TurnDirection = ETurnDirection::Right;
	else if (Value < 0.0f)
		TurnDirection = ETurnDirection::Left;
	
	LastRotationAmount = GetActorRotation().Yaw;


	if (!bFreelookEnabled)
	{
		AddControllerYawInput(Value);
	}
	else
	{
		CameraComponent->Rotate(Value, true);
	}

	//UpdateRotationRate();

	/*LastRotationAmount = RotationAmount;
	RotationAmount = GetActorRotation().Yaw;
	RotationRate = RotationAmount - LastRotationAmount;*/
}

void AFirstifyCharacter::UpdateRotationRate()
{
	/*update rotation rate*/
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Pitch = FirstPersonArms->GetComponentRotation().Pitch + CameraComponent->GetComponentRotation().Pitch;
	FRotator RotationChange = CurrentRotation - LastRotation;
	PitchRate = RotationChange.Pitch;
	RotationRate = RotationChange.Yaw;

	/*mark rotation for next update*/
	LastRotation = CurrentRotation;
}

bool AFirstifyCharacter::CanMove()
{
	if (IsDeadOrIncapacitated())
		return false;


	return true;
}


void AFirstifyCharacter::MoveForward(float Value)
{
	if (Value == 0.0f || !CanMove())
		return;

	GetCharacterMovement()->AddInputVector(GetActorForwardVector() * Value);

}

void AFirstifyCharacter::MoveRight(float Value)
{
	if (Value == 0.0f || !CanMove())
		return;

	GetCharacterMovement()->AddInputVector(GetActorRightVector() * Value);

}

void AFirstifyCharacter::OnCrouchPressed()
{
	bool bToggleCrouch = Cast<AFirstifyPlayerController>(Controller)->bToggleCrouch;
	
	/*toggle-crouch mode*/
	if (bToggleCrouch)
	{
		/*we're already crouching - so release*/
		if (bIsCrouched && bInitialCrouchKeystroke == false)
		{			
			OnCrouchReleased ();
		}
		/*not crouching*/
		else
		{
			bInitialCrouchKeystroke = true; //mark that we just pressed the crouch button (for crouch toggle check)
			Crouch();
		}
	}
	/*hold-crouch mode*/
	else
	{
		Crouch();
	}
	
}

void AFirstifyCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	CameraComponent->SetRelativeLocation(FVector(0, 0, ReturnCharacterMovement()->CrouchedHalfHeight)); //immedietaly offset our camera above the new capsule height as if it hasn't changed
	GetWorldTimerManager().SetTimer(CrouchHandler, this, &AFirstifyCharacter::LerpCrouch, 0.005f, true);
}

void AFirstifyCharacter::LerpCrouch()
{
	float NewCameraHeight = 0.0f;
	float CurrentCameraHeight = CameraComponent->GetRelativeLocation().Z;


	NewCameraHeight = FMath::Lerp(CurrentCameraHeight, 0.0f, 0.25f); //transition from current offset back to 0 to match where the capsule already is

	if (FMath::IsNearlyEqual(NewCameraHeight, 0.0f))
	{
		NewCameraHeight = 0;
		GetWorldTimerManager().ClearTimer(CrouchHandler);
	}

	CameraComponent->SetRelativeLocation(FVector(0, 0, NewCameraHeight));
}

void AFirstifyCharacter::OnCrouchReleased()
{
	if (bIsCrouched == false)
		return;

	bool bToggleCrouch = Cast<AFirstifyPlayerController>(Controller)->bToggleCrouch;

	/*toggle-crouch mode*/
	if (bToggleCrouch)
	{
		/*first time releasing crouch - so ignore*/
		if (bInitialCrouchKeystroke)
		{
			bInitialCrouchKeystroke = false;
		}
		/*we've already released the toggle keystroke - so we can uncrouch from here on out*/
		else 
		{
			UnCrouch();
		}
		
	}
	/*hold-crouch mode*/
	else
	{
		UnCrouch();
	}	
}

void AFirstifyCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	CameraComponent->SetRelativeLocation(FVector(0, 0, ReturnCharacterMovement()->CrouchedHalfHeight * -1));
	GetWorldTimerManager().SetTimer(CrouchHandler, this, &AFirstifyCharacter::LerpCrouch, 0.005f, true);
}

void AFirstifyCharacter::Sprint()
{
	if (!ReturnCharacterMovement() || !CanSprint())
		return;

	ReturnCharacterMovement()->BeginSprint();
}

void AFirstifyCharacter::StopSprinting()
{
	if (!ReturnCharacterMovement())
		return;

	ReturnCharacterMovement()->EndSprint();
}

bool AFirstifyCharacter::CanSprint()
{
	return true;
}

void AFirstifyCharacter::Jump()
{	
	if (UFirstifyCharacterMovement *CharMov = Cast<UFirstifyCharacterMovement>(GetCharacterMovement()))
	{
		if (CharMov->CanMantle())
			CharMov->PerformMantleMove();

		else if (CharMov->IsJumpAllowed())
		{
			CharMov->bWantsToMantle = true;
			Super::Jump();
		}

	}
	else //fallback if we're not using the firstify component or child of one
	{
		Super::Jump();
	}
}

void AFirstifyCharacter::StopJumping()
{
	/*firstify character implementation*/
	if (UFirstifyCharacterMovement* CharMov = Cast<UFirstifyCharacterMovement>(GetCharacterMovement()))
	{
		CharMov->bWantsToMantle = false;
	}
	/*fallback to regular implementation of non-firstify characters*/
	else
	{
		Super::StopJumping();
	}
	
}

void AFirstifyCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	float FallHeight = ReturnCharacterMovement()->FallHeight;
//	float HitVelocity = GetVelocity().Z;

	PlayLandedSound();

	if (ReturnCharacterMovement()->bCanTakeFallDamage && FallHeight < 0 && FallHeight > ReturnCharacterMovement()->FallDamageHeightThreshold)
	{
		TakeDamage(ReturnCharacterMovement()->FallDamageMin, FDamageEvent(UDamageType_FallDamage::StaticClass()), GetController(), this);
	}	
}


#pragma endregion


//==========================================
//==================DAMAGE==================
//==========================================

#pragma region DAMAGE/HEALTH


float AFirstifyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/*disable damage with GodMode on*/
	if (bGodMode)
		return 0.0f;


	/*calls InternalTakeDamage() and what nots!*/
	DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageEvent.DamageTypeClass->IsChildOf(UDamageType_FallDamage::StaticClass()))
		DamageAmount = TakeFallDamage(DamageAmount);

	Health -= DamageAmount;

	/*died*/
	if (Health <= 0 && !IsDead())
	{
		Die(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	
	MulticastOnTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);


	return DamageAmount;
}


float AFirstifyCharacter::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/*disable damage with GodMode on*/
	if (bGodMode)
		return 0.0f;

	/*get hit bone*/
	FVector* HitBoneLoc = (FVector*)0;
	FName HitBone = PointDamageEvent.HitInfo.BoneName;
	float DamageTaken = Damage;

	
	/*allow damage to be modified by body part*/
	#pragma region BODY HITS
	if (HeadBones.Contains(HitBone))
		DamageTaken = TakeHeadDamage(Damage, PointDamageEvent);
	if (NeckBones.Contains(HitBone))
		DamageTaken = TakeNeckDamage(Damage, PointDamageEvent);
	if (TorsoBones.Contains(HitBone))
		DamageTaken = TakeChestDamage(Damage, PointDamageEvent);
	if (AbdomenBones.Contains(HitBone))
		DamageTaken = TakeAbdominalDamage(Damage, PointDamageEvent);
	if (PelvisBones.Contains(HitBone))
		DamageTaken = TakePelvicDamage(Damage, PointDamageEvent);
	if (LeftArmBones.Contains(HitBone))
		DamageTaken = TakeArmDamage(Damage, PointDamageEvent, ESide::Left);
	if (RightArmBones.Contains(HitBone))
		DamageTaken = TakeArmDamage(Damage, PointDamageEvent, ESide::Right);
	if (LeftLegBones.Contains(HitBone))
		DamageTaken = TakeLegDamage(Damage, PointDamageEvent, ESide::Left);
	if (RightLegBones.Contains(HitBone))
		DamageTaken = TakeLegDamage(Damage, PointDamageEvent, ESide::Right);
	#pragma endregion

	
	/*final health modification*/
	Health -= DamageTaken;

	
	
	/*@TODO: PRETTY SURE I CAN REMOVE ALL THIS - it should be done inside TakeDamage()*/


	/*kill player if enough damage has occured*/
	//if (Health <= 0 && !IsDead())
	//	Die(DamageTaken,PointDamageEvent,EventInstigator,DamageCauser);

	
	/*player notifications*/
	//if (IsLocallyControlled())
	//{
	//	/*broadcast based on +/-*/
	//	if (Health < LastHealthUpdate && OnHealthDecrease.IsBound())
	//		OnHealthDecrease.Broadcast();
	//	else if (Health > LastHealthUpdate&& OnHealthIncrease.IsBound())
	//		OnHealthIncrease.Broadcast();

	//	/*update our last known health to current health for next update*/
	//	LastHealthUpdate = Health;
	//}

	return DamageTaken;
}

void AFirstifyCharacter::MulticastOnTakeDamage_Implementation(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/*player notifications*/
	if (IsLocallyControlled())
	{
		/*notify our controller we got hit (for easy UMG/UI hooks)*/
		if (AFirstifyPlayerController* PC = Cast<AFirstifyPlayerController>(GetController()))
			PC->OnNotifyOfPawnHit(this, DamageCauser);

		/*broadcast based on +/-*/
		if (Health < LastHealthUpdate && OnHealthDecrease.IsBound())
			OnHealthDecrease.Broadcast();
		else if (Health > LastHealthUpdate && OnHealthIncrease.IsBound())
			OnHealthIncrease.Broadcast();

		/*update our last known health to current health for next update*/
		LastHealthUpdate = Health;
	}

	/*play sound*/
	PlayPainSound(DamageEvent.DamageTypeClass);
}

/*called only when needed to keep things up to date*/
void AFirstifyCharacter::OnRep_HealthUpdate()
{
	/*broadcast basde on +/-*/
	if (Health < LastHealthUpdate && OnHealthDecrease.IsBound())
		OnHealthDecrease.Broadcast();
	else if (Health > LastHealthUpdate&& OnHealthIncrease.IsBound())
		OnHealthIncrease.Broadcast();

	/*update our last health to current health for next check*/
	LastHealthUpdate = Health;
}

float AFirstifyCharacter::TakeHeadDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent)
{	
	BodyCondition.Head.DamagePercent += DamageAmount;

	return DamageAmount;
}

float AFirstifyCharacter::TakeNeckDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent)
{	
	return DamageAmount;
}

float AFirstifyCharacter::TakeChestDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent)
{
	/*pass notification to AnimInstance for HitReactions*/
	if (UFirstifyCharacterAnimInstance* xFAnimInstance = Cast<UFirstifyCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
		xFAnimInstance->OnTakeDamage(EBodyPart::Chest, ESide::Center);

	BodyCondition.Chest.DamagePercent += DamageAmount;

	return DamageAmount;
}

float AFirstifyCharacter::TakeAbdominalDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent)
{
	BodyCondition.Abdomen.DamagePercent += DamageAmount;

	return DamageAmount;
}

float AFirstifyCharacter::TakePelvicDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent)
{
	return DamageAmount;
}

float AFirstifyCharacter::TakeArmDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent, ESide Side)
{
	

	if (Side == ESide::Left)
	{
		if (UFirstifyCharacterAnimInstance* xFAnimInstance = Cast<UFirstifyCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
			xFAnimInstance->OnTakeDamage(EBodyPart::LeftArm, ESide::Left);

		BodyCondition.LeftArm.DamagePercent += DamageAmount;
	}

	else if (Side == ESide::Right)
	{
		if (UFirstifyCharacterAnimInstance* xFAnimInstance = Cast<UFirstifyCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
			xFAnimInstance->OnTakeDamage(EBodyPart::RightArm, ESide::Right);

		BodyCondition.RightArm.DamagePercent += DamageAmount;
	}


	return DamageAmount;
}

float AFirstifyCharacter::TakeLegDamage(float DamageAmount, FPointDamageEvent const& PointDamageEvent, ESide Side)
{
	if (Side == ESide::Left)
	{
		BodyCondition.LeftLeg.DamagePercent += DamageAmount;
	}

	else if (Side == ESide::Right)
	{
		BodyCondition.RightLeg.DamagePercent += DamageAmount;
	}

	return DamageAmount;
}

float AFirstifyCharacter::TakeFallDamage(float DamageAmount)
{
	//PlayFallPainSound(); //@DEPRECIATED

	return DamageAmount;
}

//===========================================
//===================DEATH===================
//===========================================

bool AFirstifyCharacter::IsPendingDeath()
{
	if (GetWorldTimerManager().IsTimerActive(DeathAnimationTimer))
		return true;
	
	return false;
}

bool AFirstifyCharacter::IsDeadOrIncapacitated()
{
	if (HealthState == EHealthState::Incapacitated || IsDead())
		return true;
	
	return false;
}

bool AFirstifyCharacter::IsDead()
{
	if (HealthState == EHealthState::Dead || IsPendingDeath())
		return true;

	return false;
}

void AFirstifyCharacter::Die(float Damage, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	/*force health down - incase this was called from other means*/
	Health = 0;
	HealthState = EHealthState::Dead;
	EndFire();

	/*stop movement of capsule*/
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	/*stop any AI Behaviors if needed*/	
	//if (AAIController* AIC = Cast<AAIController>(GetController()))
	//{
	//	if(IsValid(AIC->GetBrainComponent()))
	//		AIC->BrainComponent->StopLogic(FString("Died"));
	//}

	/*hide first stuff*/
	if (IsLocallyControlled())
	{
		FirstPersonArms->SetVisibility(false, true);
	}

	/*get death animation*/
	UAnimSequenceBase* DeathAnim = ReturnDeathAnimation();

	
	/*notify all players of the death*/
	MulticastOnDeath(EventInstigator, DamageCauser, DamageEvent.DamageTypeClass, EBodyPart::Unknown,DeathAnim);
}

void AFirstifyCharacter::MulticastOnDeath_Implementation(AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UDamageType> DamgeType, EBodyPart BodyPartHit = EBodyPart::Unknown, class UAnimSequenceBase* DeathAnimation = nullptr)
{	
	/*controlling player adjustments*/
	if (IsLocallyControlled())
	{
		if(FirstPersonDeathSocketName != "")
			CameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform,FirstPersonDeathSocketName);
		else
			CameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform);

		/*disable movement*/
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}
	}

	/*play death animation if one is available*/
	if (DeathAnimation != nullptr && GetMesh() && GetMesh()->GetAnimInstance())
	{
		//GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(DeathAnimation, FName("DefaultSlot"));
		
		/*time the animation for switching to ragdoll*/
		float DeathAnimationLength = DeathAnimation->GetPlayLength() * 0.95; //end it just short on purpose
		GetWorldTimerManager().SetTimer(DeathAnimationTimer, this, &AFirstifyCharacter::OnDeathComplete, DeathAnimationLength, false);
		GetMesh()->PlayAnimation(DeathAnimation, false);
		//GetMesh()->GetAnimInstance()->PlaySlotAnimation(DeathAnimation, FName("DefaultSlot"));
	}
	/*otherwise - ragdoll*/
	else
	{		
		Ragdoll();
		GetWorldTimerManager().SetTimer(DeathAnimationTimer, this, &AFirstifyCharacter::OnDeathComplete, 0.25f, false);
	}

	/*notify anyone listening they just died*/
	if (OnDeathEvent.IsBound())
		OnDeathEvent.Broadcast(this,EventInstigator,DamageCauser);
}

void AFirstifyCharacter::OnDeathComplete()
{
	if(IsLocallyControlled())
	{
		CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); //detach camera to prevent ragdoll shaking
		if (bFadeCameraOnDeath)
			BeginDeathScreenFade();
	}
	
	DropInHand();
	Ragdoll();
}

void AFirstifyCharacter::BeginDeathScreenFade()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		float FadeDuration = FMath::RandRange(1.95f, 5.5f);
		PC->PlayerCameraManager->StartCameraFade(0, 1.0, FadeDuration, FLinearColor::Black, true, true);

		GetWorldTimerManager().SetTimer(DeathScreenFadeTimer, this, &AFirstifyCharacter::OnEndDeathScreenFade, FadeDuration, false);
	}
}

void AFirstifyCharacter::OnEndDeathScreenFade()
{
	/*initialize properties*/
	AFirstifyPlayerController* PC = Cast<AFirstifyPlayerController>(GetController());

	/*safety check*/
	if (!PC || !PC->PlayerCameraManager)
		return;

	PC->PlayerCameraManager->StopCameraFade();	

	if (bAutoSpectatateOnDeath)
		PC->SwitchToSpectator();
}

void AFirstifyCharacter::Suicide(AController* EventInstigator)
{
	if (EventInstigator != GetController())
		return;

	Die(10000, FDamageEvent(), EventInstigator,this);
}

UAnimSequenceBase* AFirstifyCharacter::ReturnDeathAnimation(float Damage, EBodyPart BodyPartHit, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DeathAnimationSets.Num() <= 0)
		return nullptr;

	/*loop through all death animation groups and see if something is there*/
	for (int32 i = 0; i < DeathAnimationSets.Num(); i++)
	{
		if(IsValid(DeathAnimationSets[i]))
			return DeathAnimationSets[i]->ReturnDeathAnimation(FName("Gunshot_Chest_Front"));		
	}
	
	return nullptr;
}

void AFirstifyCharacter::Ragdoll()
{
	/*safety check - bail if no mesh to ragdoll*/
	if (!GetMesh())
		return;

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();

	/*cleanup capsule*/
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->Deactivate();
	}
	/*disable movement*/
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}
}

//UMaterialInstance* AFirstifyCharacter::ReturnBloodSplatterDecal(EImpactFXSize ImpactSize)
//{
//	UMaterialInstance* MatReturn = nullptr;
//	
//	switch (ImpactSize)
//	{
//	case EImpactFXSize::Small: if (BloodSplatters_Medium.Num() > 0) { MatReturn = BloodSplatters_Small[FMath::RandRange(0, (BloodSplatters_Medium.Num() - 1))]; }
//		break;
//	case EImpactFXSize::Medium: if (BloodSplatters_Medium.Num() > 0) { MatReturn = BloodSplatters_Medium[FMath::RandRange(0, (BloodSplatters_Medium.Num() - 1))]; }
//		break;
//	case EImpactFXSize::Large: if (BloodSplatters_Medium.Num() > 0) { MatReturn = BloodSplatters_Large[FMath::RandRange(0, (BloodSplatters_Medium.Num() - 1))]; }
//		break;
//	default: if (BloodSplatters_Medium.Num() > 0) { MatReturn = BloodSplatters_Medium[FMath::RandRange(0, (BloodSplatters_Medium.Num() - 1))]; }
//		break;
//	}
//
//	return MatReturn;
//}

#pragma endregion


#pragma region UI/HUD
TSubclassOf<UUserWidget> AFirstifyCharacter::ReturnHUDClass()
{
	return HUDClass;
}

#pragma endregion



//==================================
//==============CAMERA==============
//==================================

void AFirstifyCharacter::ToggleFreelook()
{
	if (bFreelookEnabled)
		DisableFreelook();
	else
		EnableFreelook();
}

void AFirstifyCharacter::EnableFreelook()
{
	bFreelookEnabled = true;
}

void AFirstifyCharacter::DisableFreelook()
{
	bFreelookEnabled = false;
	CameraComponent->ResetFreelookRotation();
}


//=========================================
//==============POST PROCCESS==============
//=========================================

void AFirstifyCharacter::AddOrUpdatePostProcessEffect(UMaterialInterface* PostProcessMaterial, float Weight)
{
	if (!CameraComponent)
		return;

	CameraComponent->AddOrUpdateBlendable(PostProcessMaterial, Weight);
}

//===============================================
//=================SKELETAL MESH=================
//===============================================

void AFirstifyCharacter::MulticastAttachToRightHand_Implementation(AActor* ItemToAttach)
{
	
	ItemToAttach->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHandSocketName);
}

void AFirstifyCharacter::MulticastAttachToLeftHand_Implementation(AActor* ItemToAttach)
{
	ItemToAttach->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHandSocketName);
}



//====================================
//=============ANIMATIONS=============
//====================================
UAnimInstance* AFirstifyCharacter::ReturnFirstPersonAnimTree()
{
	if (FirstPersonArms)
		return FirstPersonArms->ArmsMesh->GetAnimInstance();
	
	return nullptr;
}


void AFirstifyCharacter::InitFirstPersonArms(TSubclassOf<class UFirstPersonAnimInstance> ArmsAnimBlueprint)
{
	/*safety check*/
	if (!ArmsAnimBlueprint)
	{
		return;
	}

	//FirstPersonArms->ArmsMesh->SetAnimInstanceClass(ArmsAnimBlueprint);
}

void AFirstifyCharacter::ResetFirstPersonArms()
{
	
}





//================================
//=============SOUNDS=============
//================================

void AFirstifyCharacter::PlayFootstepSound(ESide Foot)
{
	/*safety check*/
	if (!AudioComponent)
		return;

	FVector SoundLocation = GetActorLocation();
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	/*use foot location for sound spawn*/
	if (GetMesh())
	{
		if (Foot == ESide::Left && GetMesh()->GetSocketByName(LeftFootSocketName))
			SoundLocation = GetMesh()->GetSocketLocation(LeftFootSocketName);
		else if (Foot == ESide::Right && GetMesh()->GetSocketByName(RightFootSocketName))
			SoundLocation = GetMesh()->GetSocketLocation(RightFootSocketName);
	}

	USoundCue* SoundToPlay = nullptr;

	/*trace down from foot*/
	if (SurfaceFootstepSounds != nullptr)
	{
		FHitResult HitResults;
		FVector TraceStart = SoundLocation;
		FVector TraceEnd = TraceStart + ((GetActorUpVector() * -1) * 10);
		FCollisionQueryParams CollisionParams;
		CollisionParams.bReturnPhysicalMaterial = true;
		CollisionParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByProfile(HitResults, TraceStart, TraceEnd, FName("BlockAll"), CollisionParams))
		{
			SurfaceType = HitResults.PhysMaterial.Get()->SurfaceType;

			for (auto SurfaceEntry : SurfaceFootstepSounds->Sounds)
			{
				if (SurfaceEntry.SurfaceType == SurfaceType)
				{
					SoundToPlay = SurfaceEntry.SoundCue;
					break;
				}
			}			
		}
	}
	
	/*fallback check*/
	if (SoundToPlay)
		AudioComponent->SetSound(SoundToPlay);
	else
		AudioComponent->SetSound(GenericFootstep);
	
	
	AudioComponent->Play();
}

void AFirstifyCharacter::PlayPainSound(TSubclassOf<UDamageType> DamageType)
{
	/*safety check*/
	if (!PainSounds)
		return;

	for (auto PainSound : PainSounds->PainSounds)
	{
		/*play matching sound*/
		if (DamageType == PainSound.DamageType)
		{
			AudioComponent->SetSound(PainSound.Sound);
			AudioComponent->Play();
		}
	}
}


void AFirstifyCharacter::PlayLandedSound()
{
	/*safety check*/
	if (!AudioComponent)
		return;

	FVector SoundLocation = GetActorLocation();
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	USoundCue* SoundToPlay = nullptr;

	/*trace down from foot*/
	if (SurfaceLandSounds != nullptr)
	{
		FHitResult HitResults;
		FVector TraceStart = GetActorLocation();
		FVector TraceEnd = TraceStart + ((GetActorUpVector() * -1) * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 10)); //Factor in Capsule Height + add a little for tolerance
		FCollisionQueryParams CollisionParams;
		CollisionParams.bReturnPhysicalMaterial = true;
		CollisionParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByProfile(HitResults, TraceStart, TraceEnd, FName("BlockAll"), CollisionParams))
		{
			SurfaceType = HitResults.PhysMaterial.Get()->SurfaceType;

			for (auto SurfaceEntry : SurfaceLandSounds->Sounds)
			{
				if (SurfaceEntry.SurfaceType == SurfaceType)
				{
					SoundToPlay = SurfaceEntry.SoundCue;
					break;
				}
			}
		}
	}


	/*fallback check*/
	if (SoundToPlay)
		AudioComponent->SetSound(SoundToPlay);
	else
		AudioComponent->SetSound(GenericLandedSound);

	AudioComponent->Play();
}

//void AFirstifyCharacter::PlayFallPainSound()
//{
//	//if (!AudioComponent || !FallPainSound)
//	//	return;
//
//	//AudioComponent->SetSound(FallPainSound);
//	//AudioComponent->Play();
//}
//
//void AFirstifyCharacter::PlayTerminalFallSound()
//{
//	if (!AudioComponent || !TerminalFallSound)
//		return;
//
//
//}

AFirstifyPlayerController* AFirstifyCharacter::ReturnPlayerController()
{	
	return Cast<AFirstifyPlayerController>(GetController());
}

void AFirstifyCharacter::BeginDestroy()
{


	Super::BeginDestroy();	
}

void AFirstifyCharacter::Destroyed()
{
	if (GetController())
		GetController()->UnPossess();

	/*inventory cleanup*/
	if (IsValid(ReturnInHand()))
	{		
		ReturnInHand()->Destroy(true);
	}

	Super::Destroyed();
}