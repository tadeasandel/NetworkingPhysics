// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "NetworkingPhysicsHud.generated.h"


UCLASS(config = Game)
class ANetworkingPhysicsHud : public AHUD
{
	GENERATED_BODY()

public:
	ANetworkingPhysicsHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
