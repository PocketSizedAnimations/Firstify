// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/Components/FirstPersonArmsComponent.h"
#include "Players/FirstifyCharacterAnimInstance.h"
#include "Animations/FirstPersonAnimInstance.h"



UFirstPersonArmsComponent::UFirstPersonArmsComponent(const FObjectInitializer& ObjectInitializer)
{	
	//bOnlyOwnerSee = true; //allows only the controlling player to see their arms
	//SetCastShadow(false); //we want the full model to cast shadows, but not just "floating arms" to
	//SetVisibility(false); //hide it by default as it's distracting
	//bReceivesDecals = false;

	SetIsReplicatedByDefault(true);
	

	InitialCameraOffset = GetRelativeLocation(); //get the default object, and records where the offset is to be used for resetting
	SetRelativeLocation(FVector(0, 0, 80));

	ArmsMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("ArmsMeshComp"));
	if (ArmsMesh)
	{
		ArmsMesh->SetupAttachment(this);
		ArmsMesh->SetRelativeLocation(FVector(0, 0, -80));
		ArmsMesh->CastShadow = 0;


		const FString ArmsMeshPath = "/Firstify/Characters/FirstPerson/Mesh/FirstPersonArms_Mannequin_MaleA";
		USkeletalMesh* _armsMesh = ConstructorHelpers::FObjectFinder<USkeletalMesh>(*ArmsMeshPath).Object;
		if(_armsMesh)
			ArmsMesh->SkeletalMesh = _armsMesh;

		const FString AnimBlueprintPath = "/Firstify/Characters/FirstPerson/Blueprints/FirstPersonArms_MAB";
		UClass* _animInstance = ConstructorHelpers::FClassFinder<UAnimInstance>(*AnimBlueprintPath).Class;
		if (_animInstance)
			ArmsMesh->SetAnimInstanceClass(_animInstance);
	}
}

void UFirstPersonArmsComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	InitialCameraOffset = GetRelativeLocation();
	DefaultAnimBlueprint = ArmsMesh->AnimClass;
}

void UFirstPersonArmsComponent::SetArmAnimInstance(TSubclassOf<UAnimInstance> NewAnimInstance)
{
	if (!NewAnimInstance)
		return;

	ArmsMesh->SetAnimInstanceClass(NewAnimInstance);
	ArmsMesh->ResetAnimInstanceDynamics(ETeleportType::ResetPhysics);
}

void UFirstPersonArmsComponent::ResetArmAnimInstance()
{	
	ArmsMesh->SetAnimInstanceClass(DefaultAnimBlueprint);
	ArmsMesh->ResetAnimInstanceDynamics(ETeleportType::ResetPhysics);
}


void UFirstPersonArmsComponent::OffsetMesh(FVector Offset)
{
	FVector NewOffset = InitialCameraOffset + Offset;	
	SetRelativeLocation(NewOffset);
}

void UFirstPersonArmsComponent::ResetMeshOffset()
{
	SetRelativeLocation(InitialCameraOffset);
}

void UFirstPersonArmsComponent::AttachToLeftHand(USceneComponent* Component)
{
	Component->AttachToComponent(ArmsMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("LeftHandSocket"));
}

void UFirstPersonArmsComponent::AttachToRightHand(USceneComponent* Component)
{
	Component->AttachToComponent(ArmsMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
}

void UFirstPersonArmsComponent::DetachFromArms()
{

}






