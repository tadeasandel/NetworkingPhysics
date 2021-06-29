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

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		ReplicatedTransform = GetActorTransform();
	}

	DrawDebugString(GetWorld(), FVector(0,0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void ACarPawn::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinimumTurningRadius * SteeringThrow;
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

FVector ACarPawn::GetAirResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
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
	DOREPLIFETIME(ACarPawn, ReplicatedTransform);
	DOREPLIFETIME(ACarPawn, Velocity);
	DOREPLIFETIME(ACarPawn, Throttle);
	DOREPLIFETIME(ACarPawn, SteeringThrow);
}

void ACarPawn::MoveForward(float Value)
{
	Throttle = Value;
	Server_MoveForward(Value);
}

void ACarPawn::MoveRight(float Value)
{
	SteeringThrow = Value;
	Server_MoveRight(Value);
}

void ACarPawn::Server_MoveForward_Implementation(float Value)
{
	Throttle = Value;
}

bool ACarPawn::Server_MoveForward_Validate(float Value)
{
	return FMath::Abs(Value) <= 1;
}

void ACarPawn::Server_MoveRight_Implementation(float Value)
{
	SteeringThrow = Value;
}

bool ACarPawn::Server_MoveRight_Validate(float Value)
{
	return FMath::Abs(Value) <= 1;
}

void ACarPawn::OnRep_ReplicatedTransform()
{
	SetActorTransform(ReplicatedTransform);
}