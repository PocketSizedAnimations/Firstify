// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Blutilities/MapSelectionBlueprintLibrary.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include <cctype>
#include "Firstify.h"


TArray<FString> UMapSelectionBlueprintLibrary::ReturnAllMapNamesPrefixed(FString RelativeDirectory, FString RequiredPrefix)
{
	TArray<FString> MapFiles;
	FString SearchDirectory = FPaths::ProjectContentDir() + "/" + RelativeDirectory;
	FString SearchPattern = FString(RequiredPrefix + "*.umap");

	IFileManager::Get().FindFilesRecursive(MapFiles, *SearchDirectory, *SearchPattern, true, false, false);

	for (int32 i = 0; i < MapFiles.Num(); i++)
	{
		//replace the whole directory string with only the name of the map

		int32 lastSlashIndex = -1;
		if (MapFiles[i].FindLastChar('/', lastSlashIndex))
		{
			FString pureMapName;

			//length - 5 because of the ".umap" suffix
			for (int32 j = lastSlashIndex + 1; j < MapFiles[i].Len() - 5; j++)
			{
				pureMapName.AppendChar(MapFiles[i][j]);
			}

			MapFiles[i] = pureMapName;
		}
	}


	return MapFiles;
}

FString UMapSelectionBlueprintLibrary::PrettyPrintMapName(FString MapName, FString PrefixToRemove, bool bSpaceBetweenCapitilizations)
{
	MapName.RemoveFromStart(PrefixToRemove); //remove prefix
	TArray<TCHAR> CharArray = MapName.GetCharArray();
	FString PrettifiedName;

	/*loop through all characters*/
	for (int i = 0; i < CharArray.Num(); i++)
	{
		/*add a space inbetween any capitalized words*/
		if (bSpaceBetweenCapitilizations && !islower(CharArray[i]))
		{
			PrettifiedName += " ";			
		}

		PrettifiedName += CharArray[i];
	}
	
	return PrettifiedName;
}

FString UMapSelectionBlueprintLibrary::CreateLevelMetaJSON(FLevelMetaData LevelMetaData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> Objectives;
	JsonObject->SetStringField("LevelName", LevelMetaData.LevelName);
	JsonObject->SetStringField("Author", LevelMetaData.Author);
	JsonObject->SetStringField("Description", LevelMetaData.Description);
	JsonObject->SetStringField("PreviewImgPath", LevelMetaData.PreviewImgRelativePath);

	/*add all objectives*/
	for (FLevelMetaObjective Objective : LevelMetaData.Objectives)
	{
		TSharedPtr<FJsonObject> ObjectiveJson = MakeShareable(new FJsonObject);;
		ObjectiveJson->SetStringField("Objective", Objective.Objective);
		ObjectiveJson->SetStringField("Description", Objective.Description);

		Objectives.Add(MakeShareable(new FJsonValueObject(ObjectiveJson)));
	}
	
	/*add objectives array to JSON Object*/
	JsonObject->SetArrayField("Objectives", Objectives);
	
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	/*log to console our data*/
	UE_LOG(LogFirstify, Log, TEXT("UMapSelectionBlueprintLibrary::CreateLevelMetaJSON()"));
	UE_LOG(LogFirstify, Log, TEXT("%s"),*OutputString);

	return OutputString;
}

bool UMapSelectionBlueprintLibrary::SaveLevelMetaJSONFile(FString SavePath, FLevelMetaData LevelMetaData)
{
	/*remove white spaces from Level Name*/
	FString LevelName = LevelMetaData.LevelName;
	LevelName.RemoveSpacesInline();

	/*initialize*/
	FString FileName = LevelName + ".json";
	FString FilePath = FPaths::ProjectContentDir() + SavePath;
	FString FullPath = FilePath + FileName;
	FString JsonData = CreateLevelMetaJSON(LevelMetaData);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString FilePath2 = FPaths::ProjectContentDir() + "file.json";

	/*log*/
	UE_LOG(LogFirstify, Log, TEXT("SaveLevelMetaJSONFile()"));
	UE_LOG(LogFirstify, Log, TEXT("%s"), *FullPath);

	/*save file - return true if succesful*/
	return FFileHelper::SaveStringToFile(JsonData, *FullPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_NoFail);
}

/* LoadLevelMetaJSONFile() - Returns the JSON from first matching .JSON file found in search directory (or subdirectories)
*
*
*
*/
FString UMapSelectionBlueprintLibrary::LoadLevelMetaJSONFile(FString ContentSearchPath, FString LevelName)
{
	/*remove any whitespaces from Level Name*/
	LevelName.RemoveSpacesInline();
	
	/*initialize*/
	FString JsonData; 
	TArray<FString> JsonFiles;
	FString SearchDirectory = FPaths::ProjectContentDir() + ContentSearchPath;
	FString SearchPattern = LevelName + FString(".json");
	
	/*log*/
	UE_LOG(LogFirstify, Log, TEXT("UMapSelectionBlueprintLibrary::LoadLevelMetaJSONFile()"));
	UE_LOG(LogFirstify, Log, TEXT("\t SearchDirectory: %s"), *SearchDirectory);


	/*find any matching JSON files*/
	IFileManager::Get().FindFilesRecursive(JsonFiles, *SearchDirectory, *SearchPattern, true, false, false);

	UE_LOG(LogFirstify, Log, TEXT("\t JSON files found: %i"), JsonFiles.Num());
	/*validation check*/
	if (JsonFiles.Num() > 0)
	{
		FString JsonFile = JsonFiles[0]; //grab the first match

		/*load file*/
		if (IFileManager::Get().FileExists(*JsonFile))  //validation check
		{
			FFileHelper::LoadFileToString(JsonData, *JsonFile);
		}
		else
			JsonData = "FILE NOT FOUND";
	}
	else
		JsonData = "NO JSON FILE FOUND";


	return JsonData;
}

FLevelMetaData UMapSelectionBlueprintLibrary::LoadAndParseLevelMetaJSONFile()
{ 
	return FLevelMetaData();
}

FLevelMetaData UMapSelectionBlueprintLibrary::ParseLevelMetaJSON(FString JsonObjectString)
{
	FLevelMetaData LevelMeta = FLevelMetaData();
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonObjectString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		LevelMeta.LevelName = JsonObject->GetStringField("LevelName");
		LevelMeta.Author = JsonObject->GetStringField("Author");
		LevelMeta.Description = JsonObject->GetStringField("Description");
		LevelMeta.PreviewImgRelativePath = JsonObject->GetStringField("PreviewImgPath");
	}

	return LevelMeta;
}
