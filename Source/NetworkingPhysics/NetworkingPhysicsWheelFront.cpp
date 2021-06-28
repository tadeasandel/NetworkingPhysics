// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkingPhysicsWheelFront.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

UNetworkingPhysicsWheelFront::UNetworkingPhysicsWheelFront()
{
	ShapeRadius = 35.f;
	ShapeWidth = 10.0f;
	bAffectedByHandbrake = false;
	SteerAngle = 50.f;
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS

