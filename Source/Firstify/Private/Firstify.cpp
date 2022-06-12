// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Firstify.h"
#include "Firstify/FirstifyLog.h"

#define LOCTEXT_NAMESPACE "FFirstifyModule"



DEFINE_LOG_CATEGORY(LogFirstify);
DEFINE_LOG_CATEGORY(LogFirstifyAI);
DEFINE_LOG_CATEGORY(LogFirstifyAnimation);
DEFINE_LOG_CATEGORY(LogFirstifyInventory);
DEFINE_LOG_CATEGORY(LogFirstifyObjectives);
DEFINE_LOG_CATEGORY(LogWeapons);


void FFirstifyModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FFirstifyModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFirstifyModule, Firstify)