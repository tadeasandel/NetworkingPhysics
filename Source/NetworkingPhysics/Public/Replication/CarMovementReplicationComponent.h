
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

	void UpdateServerState(const FCarMove& Move);

private: 

	UPROPERTY()
	UCarMovementComponent* MovementComponent;
};
