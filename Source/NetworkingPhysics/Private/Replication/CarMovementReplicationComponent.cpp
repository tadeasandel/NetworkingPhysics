

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
		ClientTick(DeltaTime);
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

	ClientSimulatedTime += Move.DeltaTime;
	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

bool UCarMovementReplicationComponent::Server_SendMove_Validate(FCarMove Move)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;
	bool ClientNotRunningAhead = ProposedTime < GetWorld()->TimeSeconds;
	if (!ClientNotRunningAhead)
	{
		return false;
	}
	return Move.IsValid();
}

void UCarMovementReplicationComponent::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRepServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRepServerState();
		break;
	}
}

void UCarMovementReplicationComponent::AutonomousProxy_OnRepServerState()
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

void UCarMovementReplicationComponent::SimulatedProxy_OnRepServerState()
{
	if (MovementComponent == nullptr) { return; }

	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}
	ClientStartVelocity = MovementComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UCarMovementReplicationComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) { return; }
	if (MovementComponent == nullptr) { return; }

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	FHermiteCubicSpline Spline = CreateSpline();

	InterpolateLocation(Spline, LerpRatio);

	InterpolateVelocity(Spline, LerpRatio);

	InterpolateRotation(LerpRatio);
}

FHermiteCubicSpline UCarMovementReplicationComponent::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();
	return Spline;
}

void UCarMovementReplicationComponent::UpdateServerState(const FCarMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UCarMovementReplicationComponent::InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio)
{
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}
	//GetOwner()->SetActorLocation(NewLocation);
}

void UCarMovementReplicationComponent::InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	MovementComponent->SetVelocity(NewVelocity);
}

void UCarMovementReplicationComponent::InterpolateRotation(float LerpRatio)
{
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}
}

float UCarMovementReplicationComponent::VelocityToDerivative()
{
	return ClientTimeBetweenLastUpdates * 100;
}