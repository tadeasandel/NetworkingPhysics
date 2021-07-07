

#include "Pawns/CarPawn.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Replication/CarMovementReplicationComponent.h"

ACarPawn::ACarPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UCarMovementComponent>(TEXT("MovementComponent"));
	MovementReplicationComponent = CreateDefaultSubobject<UCarMovementReplicationComponent>(TEXT("MovementReplicationComponent"));
}

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

void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACarPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACarPawn::MoveRight);
}


void ACarPawn::MoveForward(float Value)
{
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SetThrottle(Value);
}

void ACarPawn::MoveRight(float Value)
{
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SetSteeringThrow(Value);
}