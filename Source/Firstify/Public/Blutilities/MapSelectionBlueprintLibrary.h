// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "MapSelectionBlueprintLibrary.generated.h"

USTRUCT(BlueprintType)
struct FLevelMetaObjective
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		FString Objective;
	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		FString Description;
};

USTRUCT(BlueprintType)
struct FLevelMetaData
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		FString LevelName;
	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		FString Author;
	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		FString Description;
	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		FString PreviewImgRelativePath;
	/*array of objectives you may wish to display*/
	UPROPERTY(BlueprintReadWrite, Category = "Level Meta")
		TArray<FLevelMetaObjective> Objectives;
};



/**
 * 
 */
UCLASS()
class FIRSTIFY_API UMapSelectionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/*returns any .umap within the directory (relative to Content folder) filtered by RequiredPrefix */
	UFUNCTION(BlueprintCallable, Category = "Level Meta")
		static TArray<FString> ReturnAllMapNamesPrefixed(FString RelativeDirectory, FString RequiredPrefix);
	/*converts a raw map file name to a prettified version for UI*/
	UFUNCTION(BlueprintCallable, Category = "Level Meta")
		static FString PrettyPrintMapName(FString MapName, FString PrefixToRemove, bool bSpaceBetweenCapitilizations = true);

	UFUNCTION(BlueprintCallable, Category = "Level Meta")
		static FString CreateLevelMetaJSON(FLevelMetaData LevelMetaData);
	UFUNCTION(BlueprintCallable, Category = "Level Meta", meta= (DisplayName="Save Level Meta JSON File"))
		static bool SaveLevelMetaJSONFile(FString SavePath, FLevelMetaData LevelMetaData);
	UFUNCTION(BlueprintCallable, Category = "Level Meta", meta = (DisplayName = "Load Level Meta JSON File"))
		static FString LoadLevelMetaJSONFile(FString ContentSearchPath, FString LevelName);
	UFUNCTION(BlueprintCallable, Category = "Level Meta", meta = (DisplayName = "Load And Parse Level Meta JSON File"))
		static FLevelMetaData LoadAndParseLevelMetaJSONFile();
	UFUNCTION(BlueprintCallable, Category = "Level Meta")
		static FLevelMetaData ParseLevelMetaJSON(FString JsonObjectString);
};
