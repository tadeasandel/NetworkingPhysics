// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkingPhysicsGameMode.h"
#include "NetworkingPhysicsPawn.h"
#include "NetworkingPhysicsHud.h"

ANetworkingPhysicsGameMode::ANetworkingPhysicsGameMode()
{
	DefaultPawnClass = ANetworkingPhysicsPawn::StaticClass();
	HUDClass = ANetworkingPhysicsHud::StaticClass();
}
