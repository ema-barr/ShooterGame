// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	virtual float GetMaxSpeed() const override;


	class FSavedMove_Shooter : public FSavedMove_Character {
	public:

		typedef FSavedMove_Character Super;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		//Teleport
		uint8 bSavedWantsToTeleport : 1;
		FVector SavedTargetTeleportPosition;
	};

	class FNetworkPredictionData_Client_Shooter : public FNetworkPredictionData_Client_Character {
	public:
		FNetworkPredictionData_Client_Shooter(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector & OldVelocity);


	uint8 bWantsToTeleport : 1;
	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_TeleportPosition(const FVector& TargetPosition);

	//Trigger the teleport ability (Called from the Client)
	UFUNCTION(BlueprintCallable, Category = "Teleport")
		void TeleportForward(float DistTeleport);

	FVector TargetTeleportPosition;
};

