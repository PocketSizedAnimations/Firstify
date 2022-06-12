// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectiveInline.generated.h"

/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew, abstract, Blueprintable, BlueprintType)
class FIRSTIFY_API UObjectiveInlineConstructor : public UObject
{
	GENERATED_BODY()
protected:

    UPROPERTY(EditAnywhere, Category = "Objetive")
        FText Name;
    UPROPERTY(EditAnywhere, Category = "Objetive")
        FText Description;


};
