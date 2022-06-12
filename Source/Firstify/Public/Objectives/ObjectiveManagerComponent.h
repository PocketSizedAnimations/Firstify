// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objectives/ObjectiveInline.h"
#include "Objectives/ObjectiveInfo.h"
#include "ObjectiveManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveAdded, AObjectiveInfo*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, AObjectiveInfo*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveFailed, AObjectiveInfo*, Objective);

UCLASS( ClassGroup=(Objectives), meta=(BlueprintSpawnableComponent) )
class FIRSTIFY_API UObjectiveManagerComponent : public UActorComponent
{
	GENERATED_BODY()
protected:

	UPROPERTY(Replicated)
		TArray<AObjectiveInfo*> Objectives;
	UPROPERTY(Instanced, EditAnywhere, Category = "Objectives")
		TArray<class UObjectiveInlineConstructor*> DefaultObjectives;

public:
	UPROPERTY(BlueprintAssignable)
		FOnObjectiveAdded OnObjectiveAdded;
	UPROPERTY(BlueprintAssignable)
		FOnObjectiveCompleted OnObjectiveCompleted;
	UPROPERTY(BlueprintAssignable)
		FOnObjectiveFailed OnObjectiveFailed;

	//=============================================================================
	//==================================FUNCTIONS==================================
	//=============================================================================

public:	
	// Sets default values for this component's properties
	UObjectiveManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Objcetives")
		AObjectiveInfo* AddObjective(AObjectiveInfo* Objective);
	UFUNCTION(BlueprintCallable, Category = "Objcetives")
		AObjectiveInfo* AddObjectiveByInfo(TSubclassOf<class AObjectiveInfo> Objective);
	UFUNCTION(BlueprintCallable, Category = "Objcetives")
		void CompleteObjective(AObjectiveInfo* Objective);
	UFUNCTION(BlueprintCallable, Category = "Objcetives", meta = (DisplayName ="Complete Objective - By Name"))
		void CompleteObjectiveByName(FName ObjectiveName);
	UFUNCTION(BlueprintCallable, Category = "Objcetives")
		void FailObjective(AObjectiveInfo* Objective);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objcetives")
		AObjectiveInfo* ReturnObjective(FName ObjectiveName);
	
};
