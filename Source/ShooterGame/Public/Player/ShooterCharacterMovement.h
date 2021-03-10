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

		//Jetpack
		uint8 bSavedWantsToUseJetpack : 1;
		FVector SavedImpulseVector;

		//Rewind
		uint8 bSavedWantsToRewind : 1;
		FVector SavedPositionRewind;
		FQuat SavedRotationRewind;

		//WallJump
		uint8 bSavedWantsToWallJump : 1;
		FVector SavedWallJumpDirection;
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


	/** Teleport */
	uint8 bWantsToTeleport : 1;
	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_TeleportPosition(const FVector& TargetPosition);

	//Trigger the teleport ability (Called from the Client)
	UFUNCTION(BlueprintCallable, Category = "Teleport")
		void TeleportForward(float DistTeleport);

	FVector TargetTeleportPosition;

	/** Jetpack */
	uint8 bWantsToUseJetpack : 1;

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_UseJetpack(const FVector& NewImpulseVector);

	//Trigger the use of jetpack (Called from the Client)
	UFUNCTION(BlueprintCallable, Category = "Jetpack")
		void UseJetpack(float StrengthJetpack);

	FVector ImpulseVector;

	// Variable that store the previous value of Air Control
	float PrevAirControlValue;


	/** Rewind */
	uint8 bWantsToRewind : 1;

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_ActiveRewind(const FVector& NewPositionRewind, const FQuat& NewRotationRewind);

	//Trigger the use of rewind (Called from the Client)
	UFUNCTION(BlueprintCallable, Category = "Rewind")
		void ActiveRewind(FVector& NewPositionRewind, FQuat& NewRotationRewind);

	FVector PositionRewind;
	FQuat RotationRewind;

	/** Wall Jump */
	uint8 bWantsToWallJump : 1;

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_WallJump(const FVector& NewWallJumpDirection);

	//Trigger the wall jump (Called from the Client)
	UFUNCTION(BlueprintCallable, Category = "WallJump")
		void WallJump(FVector& NewWallJumpDirection);

	FVector WallJumpDirection;
};

