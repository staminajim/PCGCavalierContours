// Copyright (c) 2025 James Vanas
// MIT License – see LICENSE file for details.

#include "PCGCavalierContours.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FPCGCavalierContoursModule"

void FPCGCavalierContoursModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("PCGCavalierContours module has been loaded"));
}

void FPCGCavalierContoursModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("PCGCavalierContours module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPCGCavalierContoursModule, PCGCavalierContours)
