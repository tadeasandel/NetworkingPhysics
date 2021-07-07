
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Movement/CarMovementComponent.h"
#include "CarPawn.generated.h"

class UCarMovementReplicationComponent;

UCLASS()
class NETWORKINGPHYSICS_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	ACarPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:


	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(VisibleAnywhere)
	UCarMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	UCarMovementReplicationComponent* MovementReplicationComponent;

};
