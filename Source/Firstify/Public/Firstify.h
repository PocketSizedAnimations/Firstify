// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

//========================LOGGING========================
#define GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetNameStringByIndex((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )

//**Creates a macro to easily log whether something is running on a Server/Client
#define NETMODE_WORLD (((GEngine == nullptr) || (GetWorld() == nullptr)) ? TEXT("") \
        : (GEngine->GetNetMode(GetWorld()) == NM_Client) ? TEXT("[Client] ") \
        : (GEngine->GetNetMode(GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
        : (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
        : TEXT("[Standalone] "))

DECLARE_LOG_CATEGORY_EXTERN(LogFirstify, Log, Log);
DECLARE_LOG_CATEGORY_EXTERN(LogFirstifyAI, Log, Log);
DECLARE_LOG_CATEGORY_EXTERN(LogFirstifyAnimation, Log, Log);
DECLARE_LOG_CATEGORY_EXTERN(LogFirstifyInventory, Log, Log);
DECLARE_LOG_CATEGORY_EXTERN(LogFirstifyObjectives, Log, Log);
DECLARE_LOG_CATEGORY_EXTERN(LogWeapons, Log, Log);



class FFirstifyModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
