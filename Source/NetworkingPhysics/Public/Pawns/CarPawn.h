// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CarPawn.generated.h"

USTRUCT()
struct FCarMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

USTRUCT()
struct FCarState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FCarMove LastMove;
};

UCLASS()
class NETWORKINGPHYSICS_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACarPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FCarState ServerState;

	TArray<FCarMove> UnacknowledgedMoves;

	UPROPERTY()
	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float MinimumTurningRadius = 10;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

private:

	void SimulateMove(const FCarMove& Move);

	FCarMove CreateMove(float DeltaTime);
	void ClearAcknowledgedMoves(FCarMove LastMove);

	FVector GetAirResistance();
	FVector GetRollingResistance();

	void MoveForward(float Value);
	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FCarMove Move);

	UFUNCTION()
	void OnRep_ServerState();

	void UpdateLocationFromVelocity(float DeltaTime);
	void ApplyRotation(float DeltaTime, float LocalSteeringThrow);
};
