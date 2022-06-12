// Fill out your copyright notice in the Description page of Project Settings.


/*core*/
#include "Players/Components/FirstifyCharacterMovement.h"
#include "GameFramework/Character.h"
#include "UObject/Object.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

/*debug*/
#include "Firstify.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"

/*collision*/
#include "Components/CapsuleComponent.h"
#include "CollisionQueryParams.h"



void UFirstifyCharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFirstifyCharacterMovement, bWantsToSprint);
}

UFirstifyCharacterMovement::UFirstifyCharacterMovement()
{
	/*jumping*/
	MovementState.bCanJump = true;
	JumpZVelocity = 640;
	
	/*falling*/
	bCanTakeFallDamage = true;
	FallDamageHeightThreshold = 1600;
	FallDamageMin = 30;
	FallDamageMax = 70;
	TerminalVelocity = 3200;
	FallHeight = 0;

	/*base*/
	bIgnoreBaseRotation = true;
	bCanWalkOffLedges = true;
	bCanWalkOffLedgesWhenCrouching = true;

	/*stances*/
	NavAgentProps.bCanCrouch = true;
	 

	/*sprinting*/
	bCanSprint = true;
	bWantsToSprint = false;
	MaxSprintSpeed = 720;
	SprintEaseInRate = 0.25f;
	SprintEaseOutRate = 0.35f;

	/*mantling*/
	bCanMantle = true;
	MaxMantleHeight = 100;
	MaxMantleDistance = 30;

	bShowDebug = false;
}

void UFirstifyCharacterMovement::InitializeComponent()
{
	Super::InitializeComponent();

}


void UFirstifyCharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*movement input updates*/
	MovementInputAmount = (GetCurrentAcceleration().Size() / GetMaxAcceleration());
	if (HasMovementInput())
		LastMovementInputRotation = GetCurrentAcceleration().ToOrientationRotator();
	
	/*speed updates*/
	
	FVector HorizontalVelocity = Velocity; 
	HorizontalVelocity.Z = 0.0f; //ignore vertical movement (falling)
	
	if (HorizontalVelocity.Size() > 1.0f)
		bMoving = true;

	/*movement-dependant updates*/
	if (bMoving)
	{		
		LastVelocityRotation = Velocity.ToOrientationRotator();
		//Acceleration = (Velocity - PreviousVelocity) / DeltaTime;
	}

	if (CharacterOwner->IsLocallyControlled())
		MantleCheck();

	/*cache for next frame update*/
	PreviousVelocity = Velocity;
}


//==============================================================
//===========================MOVEMENT===========================
//==============================================================


float UFirstifyCharacterMovement::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		if (IsCrouching())
		{
			return MaxWalkSpeedCrouched;
		}
		else
		{
			if (IsSprinting())
				return MaxSprintSpeed;
			else
				return MaxWalkSpeed;
		}
	case MOVE_Falling:
		return MaxWalkSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}


bool UFirstifyCharacterMovement::IsSprinting() const
{
	if (CanSprint() && bWantsToSprint)
		return true;

	return false;
}

bool UFirstifyCharacterMovement::CanSprint() const
{
	/*make sure this components ability is correct*/
	if (!CanEverSprint())
		return false;

	/*make sure we're in the walking state*/
	if (MovementMode != EMovementMode::MOVE_Walking || !IsMovingOnGround())
		return false;

	
	return true;
}

void UFirstifyCharacterMovement::BeginSprint()
{
	if (!CanSprint() || IsSprinting())
		return;

	bWantsToSprint = true;
	ServerNotifyBeginSprint();
}

void UFirstifyCharacterMovement::EndSprint()
{
	if (bWantsToSprint)
	{
		bWantsToSprint = false;
		ServerNotifyEndSprint();
	}
}

bool UFirstifyCharacterMovement::ServerNotifyBeginSprint_Validate()
{
	return true;
}

void UFirstifyCharacterMovement::ServerNotifyBeginSprint_Implementation()
{
	bWantsToSprint = true;
}

bool UFirstifyCharacterMovement::ServerNotifyEndSprint_Validate()
{
	return true;
}

void UFirstifyCharacterMovement::ServerNotifyEndSprint_Implementation()
{
	bWantsToSprint = false;
}

//====================================================
//=================FALLING/JUMPING====================
//====================================================

void UFirstifyCharacterMovement::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	/*mark the final distance we fell*/
	FallHeight = GetActorLocation().Z - FallStartZLocation; //subtract where are now from where we were to see how far we've fallen

	/*finish with parent implementation*/
	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

//==========================================================================
//=================================MANTLING=================================
//==========================================================================

void UFirstifyCharacterMovement::SetWantsToMantle(bool bDesiresMantle)
{
	ServerSetWantsToMantle(bDesiresMantle);
	bWantsToMantle = bDesiresMantle;	
}

bool UFirstifyCharacterMovement::ServerSetWantsToMantle_Validate(bool bDesiresMantle)
{
	return true;
}

void UFirstifyCharacterMovement::ServerSetWantsToMantle_Implementation(bool bDesiresMantle)
{
	bWantsToMantle = bDesiresMantle;
}

void UFirstifyCharacterMovement::PerformMantleMove()
{
	if (!CanMantle())
		return;

	/*switch on mantling*/
	SetWantsToMantle(true);
	bMantling = true;

	CharacterOwner->SetActorLocation(MantleToLocation, false, nullptr, ETeleportType::ResetPhysics);


	/*reset this automatically on completion*/
	SetWantsToMantle(false);
	bMantling = false;
}

bool UFirstifyCharacterMovement::CanMantle()
{
	/*does not have the ability to mantle - so reject it*/
	if (!CanEverMantle())
		return false;
	/*already mantling - so reject it!*/
	if (bMantling)
		return false;



	if (bCanMantleToStand || bCanMantleToCrouch)
		return true;
	else
		return false;
}

/* MantleCheck() - checks to see if we can do any sort of mantling/vaulting at all
* 
*
*
*/
void UFirstifyCharacterMovement::MantleCheck()
{
	/*stop if this character is not allowed to mantle*/
	if (!CharacterOwner || !CharacterOwner->IsLocallyControlled() || !CanEverMantle())
		return;

	/*references*/
	UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();

	/*we assume we can't mantle unless the trace tells us otherwise*/
	bCanMantleToStand = false;
	bCanMantleToCrouch = false;
	ObstacleHeight = 0;

	FHitResult HitResults;

	/*check to see if anything is blocking us*/
	if (HasObstacleInFront(HitResults))
	{
		
		/*determine how tall the object is*/
		ObstacleHeight = CalcObstacleHeight();
		
		if(bShowDebug)
			UE_LOG(LogFirstify, Log, TEXT("Obstacle Height: %i"), ObstacleHeight);	
		
		FVector StandTraceStart = Capsule->GetComponentLocation() + (Capsule->GetUpVector() * ObstacleHeight); //take the obstacle's height, and push it up as high as the object
		FVector DestinationLoc = StandTraceStart + (Capsule->GetForwardVector() * (Capsule->GetScaledCapsuleRadius() * 2));
		if (CanMantleToStand(DestinationLoc))
		{
			bCanMantleToStand = true;			
			MantleToLocation = DestinationLoc; //updated our desired Mantle Location
		}



		/*auto-mantle if appropriate*/
		if (bWantsToMantle)
			PerformMantleMove();
	}
}



bool UFirstifyCharacterMovement::HasObstacleInFront(FHitResult HitResults)
{
	/*property setup*/
	UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent(); //we want to do all our traces off of the Capsule (which controls the approximate spacing) rather than the full mesh
	FVector TraceStart = Capsule->GetComponentLocation(); //Start from center of capsule
	FVector TraceDir = Capsule->GetForwardVector(); //capsule facing direction
	float TraceDistance = Capsule->GetScaledCapsuleRadius() * 2; //we want the radus (center to edge) distance so we can trace one thickness infront of us
	float TraceRadius = Capsule->GetScaledCapsuleRadius();
	float TraceHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	if (CapsuleTrace(TraceStart, TraceDir, TraceDistance, TraceHalfHeight, TraceRadius, HitResults))
	{
		ObstacleBlockingLoc = HitResults.Location;
		return true;
	}
	else
	{
		ObstacleBlockingLoc = FVector(0, 0, 0);
		return false;
	}
}

bool UFirstifyCharacterMovement::CanMantleToStand(FVector Location)
{
	UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	FVector StartLoc = Capsule->GetComponentLocation() + (Capsule->GetUpVector() * ObstacleHeight);
	FHitResult HitResults;
	
	
	if (CapsuleTrace(StartLoc, Capsule->GetForwardVector(), FVector::Distance(StartLoc, Location), Capsule->GetScaledCapsuleHalfHeight(), Capsule->GetScaledCapsuleRadius(), HitResults, bShowDebug))
	{
		return false;
	}
	else
	{
		return true;
	}
	
}



/*CalcMantlingHeight() - determine how high the object is infront of us*/
uint32 UFirstifyCharacterMovement::CalcObstacleHeight()
{
	/*reference grabbing*/
	UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();

	/*trace configuration*/
	int8 InitialTraceZOffset = Capsule->GetScaledCapsuleHalfHeight(); //we want to start at the very bottom
	int8 UnitTraceSize = 10; //divides the trace up per x unreal units

	/*trace initialization*/
	int32 TraceStep = FMath::RoundHalfToEven(float((Capsule->GetScaledCapsuleHalfHeight() * 2) + MaxMantleHeight) / float(UnitTraceSize)); //takes the highest we can mantle, and divides it by the desired units to determine how many traces we need to do
	int32 TraceIndex = 0; //iterator of trace-steps
	FVector TraceStart = (Capsule->GetComponentLocation() + (Capsule->GetForwardVector() * Capsule->GetScaledCapsuleRadius())) + (Capsule->GetUpVector() * (InitialTraceZOffset * -1));	 //Offset from center (X:Move to edge of capsule radius - Z: Move to pre-defined -Z Offset)
	float TraceDistance = Capsule->GetScaledCapsuleRadius() + MaxMantleDistance;

	ObstacleHeight = 0; //how high the unit is - defaults to 0uu

	/*trace in steps bottom-up to find the approximate size of our object*/
	for (TraceIndex; TraceIndex <= TraceStep; TraceIndex++)
	{
		FHitResult HitResults;
		FVector TraceEnd = TraceStart + (Capsule->GetForwardVector() * TraceDistance);
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(CharacterOwner);

		if (GetWorld()->LineTraceSingleByProfile(HitResults, TraceStart, TraceEnd, FName("Pawn"), CollisionParams))
		{
			if(bShowDebug)
				DrawDebugLine(GetWorld(), TraceStart, HitResults.Location, FColor::Red);
		}
		else
		{
			if(bShowDebug)
				DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green);	
			
			/*mark our obstacle height - and break out*/
			ObstacleHeight = UnitTraceSize * TraceIndex; //set the height to our current step
			break;
		}
		
		/*prepare for next loop*/
		TraceStart += Capsule->GetUpVector() * UnitTraceSize; //update the trace start upwards for next iteration
	}
	
	
	/*will default to 0 if the object is undefined or taller than we can climb - so we can safely say we can't mantle currently*/
	return ObstacleHeight;
}

bool UFirstifyCharacterMovement::CapsuleTrace(FVector StartLoc, FVector TraceDir, float Distance, float TraceHalfHeight, float TraceRadius, FHitResult& OutHitResult, bool bDrawDebug, FColor NoHitColor, FColor HitColor)
{
	FVector EndLoc = StartLoc + (TraceDir * Distance);
	FQuat CapsuleDirection = CharacterOwner->GetCapsuleComponent()->GetForwardVector().ToOrientationQuat();
	FCollisionObjectQueryParams CollisionObjectQuery;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CharacterOwner);
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(TraceRadius, TraceHalfHeight);



	/*perform trace and do some snuff with the hit*/
	
	/*hit*/
	if(GetWorld()->SweepSingleByProfile(OutHitResult, StartLoc, EndLoc,CapsuleDirection,FName("Pawn"),CapsuleShape,CollisionParams))
	{
		if(bDrawDebug)
			DrawDebugCapsule(GetWorld(), OutHitResult.Location, TraceHalfHeight, TraceRadius, CapsuleDirection, HitColor);


		return true;
	}
	/*no hit*/
	else
	{
		if(bDrawDebug)
			DrawDebugCapsule(GetWorld(), EndLoc, TraceHalfHeight, TraceRadius, CapsuleDirection, NoHitColor);


		return false;
	}
}

void UFirstifyCharacterMovement::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	/*entered falling state*/
	if (MovementMode == MOVE_Falling)
	{
		FallStartZLocation = GetActorLocation().Z; //location at which we began our fall to compare OnLand()
	}
	
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

