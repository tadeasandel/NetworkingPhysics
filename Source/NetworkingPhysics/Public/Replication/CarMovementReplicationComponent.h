
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Movement/CarMovementComponent.h"
#include "CarMovementReplicationComponent.generated.h"

USTRUCT()
struct FCarState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FCarMove LastMove;
};

struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector StartDerivative;
	FVector TargetLocation;
	FVector TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( Blueprintable, BlueprintType )
class NETWORKINGPHYSICS_API UCarMovementReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCarMovementReplicationComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void ClearAcknowledgedMoves(FCarMove LastMove);

private:

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FCarState ServerState;

	TArray<FCarMove> UnacknowledgedMoves;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FCarMove Move);

	UFUNCTION()
	void OnRep_ServerState();

	void SimulatedProxy_OnRepServerState();
	void AutonomousProxy_OnRepServerState();

	void ClientTick(float DeltaTime);

	FHermiteCubicSpline CreateSpline();
	void UpdateServerState(const FCarMove& Move);

	void InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio);
	void InterpolateRotation(float LerpRatio);
	float VelocityToDerivative();

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root; }

private:

	UPROPERTY()
	UCarMovementComponent* MovementComponent;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;

	float ClientSimulatedTime;
};
