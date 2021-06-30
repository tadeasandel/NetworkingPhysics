// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/CarPawn.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACarPawn::ACarPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

FString GetEnumText(ENetRole NetRole)
{
	switch (NetRole)
	{
	case ROLE_None:
		return TEXT("None");
	case ROLE_SimulatedProxy:
		return TEXT("SimulatedProxy");
	case ROLE_AutonomousProxy:
		return TEXT("AutonomousProxy");
	case ROLE_Authority:
		return TEXT("Authority");
	default:
		return TEXT("ERROR");
	}
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		FCarMove Move = CreateMove(DeltaTime);
		SimulateMove(Move);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}
	if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
	{
		FCarMove Move = CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void ACarPawn::ApplyRotation(float DeltaTime, float LocalSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinimumTurningRadius * LocalSteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorLocalRotation(RotationDelta);
}

void ACarPawn::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult HitResult;

	AddActorWorldOffset(Translation, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACarPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACarPawn::MoveRight);
}

void ACarPawn::SimulateMove(const FCarMove& Move)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FCarMove ACarPawn::CreateMove(float DeltaTime)
{
	FCarMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->TimeSeconds;
	return Move;
}

void ACarPawn::ClearAcknowledgedMoves(FCarMove LastMove)
{
	TArray<FCarMove> NewMoves;

	for (const FCarMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

FVector ACarPawn::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector ACarPawn::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void ACarPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACarPawn, ServerState);
}

void ACarPawn::MoveForward(float Value)
{
	Throttle = Value;
}

void ACarPawn::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void ACarPawn::Server_SendMove_Implementation(FCarMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;

	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

bool ACarPawn::Server_SendMove_Validate(FCarMove Move)
{
	return true;
}

void ACarPawn::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FCarMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}