// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToTeleport = (Flags&FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client * UShooterCharacterMovement::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);
	//check(PawnOwner->GetLocalRole() < ROLE_Authority);

	if (!ClientPredictionData) {
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Shooter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UShooterCharacterMovement::OnMovementUpdated(float DeltaTime, const FVector & OldLocation, const FVector & OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner) {
		return;
	}

	if (bWantsToTeleport) {
		bWantsToTeleport = false;

		CharacterOwner->SetActorLocation(TargetTeleportPosition, true);
	}
}



bool UShooterCharacterMovement::FSavedMove_Shooter::CanCombineWith(const FSavedMovePtr & NewMove, ACharacter * Character, float MaxDelta) const
{
	if (bSavedWantsToTeleport != ((FSavedMove_Shooter*)&NewMove)->bSavedWantsToTeleport) {
		return false;
	}
	if (SavedTargetTeleportPosition != ((FSavedMove_Shooter*)&NewMove)->SavedTargetTeleportPosition) {
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UShooterCharacterMovement::FSavedMove_Shooter::Clear()
{
	Super::Clear();

	bSavedWantsToTeleport = false;
	SavedTargetTeleportPosition = FVector::ZeroVector;
}

uint8 UShooterCharacterMovement::FSavedMove_Shooter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToTeleport) {
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UShooterCharacterMovement::FSavedMove_Shooter::SetMoveFor(ACharacter * Character, float InDeltaTime, FVector const & NewAccel, FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement) {
		bSavedWantsToTeleport = CharacterMovement->bWantsToTeleport;
		SavedTargetTeleportPosition = CharacterMovement->TargetTeleportPosition;
	}
}

void UShooterCharacterMovement::FSavedMove_Shooter::PrepMoveFor(ACharacter * Character)
{
	Super::PrepMoveFor(Character);

	UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement) {
		CharacterMovement->TargetTeleportPosition = SavedTargetTeleportPosition;
	}
}

UShooterCharacterMovement::FNetworkPredictionData_Client_Shooter::FNetworkPredictionData_Client_Shooter(const UCharacterMovementComponent & ClientMovement)
	:Super(ClientMovement)
{
}

FSavedMovePtr UShooterCharacterMovement::FNetworkPredictionData_Client_Shooter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Shooter());
}

bool UShooterCharacterMovement::Server_TeleportPosition_Validate(const FVector & TargetPosition)
{
	return true;
}

void UShooterCharacterMovement::Server_TeleportPosition_Implementation(const FVector & TargetPosition)
{
	TargetTeleportPosition = TargetPosition;
}

void UShooterCharacterMovement::TeleportForward(float DistTeleport) {
	if (PawnOwner->IsLocallyControlled()) {
		TargetTeleportPosition = PawnOwner->GetActorLocation() + PawnOwner->GetActorForwardVector() * DistTeleport;
		Server_TeleportPosition(TargetTeleportPosition);
	}

	bWantsToTeleport = true;
}
