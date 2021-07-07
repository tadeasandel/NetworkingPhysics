
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarMovementComponent.generated.h"

USTRUCT()
struct FCarMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

UCLASS(Blueprintable, BlueprintType)
class NETWORKINGPHYSICS_API UCarMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCarMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float MinimumTurningRadius = 10;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	UPROPERTY()
	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	FCarMove LastMove;

private:

	FVector GetAirResistance();
	FVector GetRollingResistance();

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float LocalSteeringThrow);
		
public:

	FORCEINLINE void SetVelocity(FVector Value) { Velocity = Value; }
	FORCEINLINE FVector GetVelocity() { return Velocity; }

	FORCEINLINE void SetThrottle(float Value) { Throttle = Value; }

	FORCEINLINE void SetSteeringThrow(float Value) { SteeringThrow = Value; }

	FORCEINLINE FCarMove GetLastMove() { return LastMove; }

	void SimulateMove(const FCarMove& Move);

private:

	FCarMove CreateMove(float DeltaTime);
};
