

#include "Replication/CarMovementReplicationComponent.h"
#include "Movement/CarMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCarMovementReplicationComponent::UCarMovementReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}

void UCarMovementReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UCarMovementComponent>();
}

void UCarMovementReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCarMovementReplicationComponent, ServerState);
}

void UCarMovementReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) { return; }

	FCarMove LastMove = MovementComponent->GetLastMove();

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn->IsLocallyControlled())
	{
		UpdateServerState(LastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
}

void UCarMovementReplicationComponent::ClearAcknowledgedMoves(FCarMove LastMove)
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

void UCarMovementReplicationComponent::Server_SendMove_Implementation(FCarMove Move)
{
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

bool UCarMovementReplicationComponent::Server_SendMove_Validate(FCarMove Move)
{
	return true;
}

void UCarMovementReplicationComponent::OnRep_ServerState()
{
	if (MovementComponent == nullptr) { return; }
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FCarMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UCarMovementReplicationComponent::UpdateServerState(const FCarMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}