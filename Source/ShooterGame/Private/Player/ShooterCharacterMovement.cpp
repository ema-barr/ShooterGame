// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrevAirControlValue = 0.f;
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
	bWantsToUseJetpack = (Flags&FSavedMove_Character::FLAG_Custom_1) != 0;
	bWantsToRewind = (Flags&FSavedMove_Character::FLAG_Custom_2) != 0;
	bWantsToWallJump = (Flags&FSavedMove_Character::FLAG_Custom_3) != 0;
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

	if (bWantsToUseJetpack) {
		bWantsToUseJetpack = false;

		PrevAirControlValue = this->AirControl;
		this->AirControl = 1.f;
		this->AddImpulse(ImpulseVector);
		Cast<AShooterCharacter>(CharacterOwner)->ConsumeFuelJetpack();
	}
	else {
		Cast<AShooterCharacter>(CharacterOwner)->RechargeFuelJetpack();
		this->AirControl = PrevAirControlValue;
	}

	if (bWantsToRewind) {
		bWantsToRewind = false;

		CharacterOwner->SetActorLocation(PositionRewind, false);
		CharacterOwner->GetController()->SetControlRotation(RotationRewind.Rotator());
	}

	if (bWantsToWallJump) {
		bWantsToWallJump = false;

		CharacterOwner->LaunchCharacter(WallJumpDirection, true, true);
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

	if (bSavedWantsToUseJetpack != ((FSavedMove_Shooter*)&NewMove)->bSavedWantsToUseJetpack) {
		return false;
	}
	if (SavedImpulseVector != ((FSavedMove_Shooter*)&NewMove)->SavedImpulseVector) {
		return false;
	}

	if (bSavedWantsToRewind != ((FSavedMove_Shooter*)&NewMove)->bSavedWantsToRewind) {
		return false;
	}
	if (SavedPositionRewind != ((FSavedMove_Shooter*)&NewMove)->SavedPositionRewind) {
		return false;
	}
	if (SavedRotationRewind != ((FSavedMove_Shooter*)&NewMove)->SavedRotationRewind) {
		return false;
	}

	if (bSavedWantsToWallJump != ((FSavedMove_Shooter*)&NewMove)->bSavedWantsToWallJump) {
		return false;
	}
	if (SavedWallJumpDirection != ((FSavedMove_Shooter*)&NewMove)->SavedWallJumpDirection) {
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UShooterCharacterMovement::FSavedMove_Shooter::Clear()
{
	Super::Clear();

	bSavedWantsToTeleport = false;
	SavedTargetTeleportPosition = FVector::ZeroVector;

	bSavedWantsToUseJetpack = false;
	SavedImpulseVector = FVector::ZeroVector;

	bSavedWantsToRewind = false;
	SavedPositionRewind = FVector::ZeroVector;
	SavedRotationRewind = FQuat::Identity;

	bSavedWantsToWallJump = false;
	SavedWallJumpDirection = FVector::ZeroVector;
}

uint8 UShooterCharacterMovement::FSavedMove_Shooter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToTeleport) {
		Result |= FLAG_Custom_0;
	}

	if (bSavedWantsToUseJetpack) {
		Result |= FLAG_Custom_1;
	}

	if (bSavedWantsToRewind) {
		Result |= FLAG_Custom_2;
	}

	if (bSavedWantsToWallJump) {
		Result |= FLAG_Custom_3;
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

		bSavedWantsToUseJetpack = CharacterMovement->bWantsToUseJetpack;
		SavedImpulseVector = CharacterMovement->ImpulseVector;

		bSavedWantsToRewind = CharacterMovement->bWantsToRewind;
		SavedPositionRewind = CharacterMovement->PositionRewind;
		SavedRotationRewind = CharacterMovement->RotationRewind;

		bSavedWantsToWallJump = CharacterMovement->bWantsToWallJump;
		SavedWallJumpDirection = CharacterMovement->WallJumpDirection;
	}
}

void UShooterCharacterMovement::FSavedMove_Shooter::PrepMoveFor(ACharacter * Character)
{
	Super::PrepMoveFor(Character);

	UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement) {
		CharacterMovement->TargetTeleportPosition = SavedTargetTeleportPosition;

		CharacterMovement->ImpulseVector = SavedImpulseVector;

		CharacterMovement->PositionRewind = SavedPositionRewind;
		CharacterMovement->RotationRewind = SavedRotationRewind;

		CharacterMovement->WallJumpDirection = SavedWallJumpDirection;
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

bool UShooterCharacterMovement::Server_UseJetpack_Validate(const FVector & NewImpulseVector)
{
	return true;
}

void UShooterCharacterMovement::Server_UseJetpack_Implementation(const FVector & NewImpulseVector)
{
	ImpulseVector = NewImpulseVector;
}


void UShooterCharacterMovement::UseJetpack(float StrengthJetpack)
{
	if (PawnOwner->IsLocallyControlled()) {
		ImpulseVector = PawnOwner->GetActorUpVector() * StrengthJetpack;
		Server_UseJetpack(ImpulseVector);
	}

	bWantsToUseJetpack = true;
}

bool UShooterCharacterMovement::Server_ActiveRewind_Validate(const FVector& NewPositionRewind, const FQuat& NewRotationRewind)
{
	return true;
}

void UShooterCharacterMovement::Server_ActiveRewind_Implementation(const FVector& NewPositionRewind, const FQuat& NewRotationRewind)
{
	PositionRewind = NewPositionRewind;
	RotationRewind = NewRotationRewind;
}

void UShooterCharacterMovement::ActiveRewind(FVector& NewPositionRewind, FQuat& NewRotationRewind)
{
	if (PawnOwner->IsLocallyControlled()) {
		PositionRewind = NewPositionRewind;
		RotationRewind = NewRotationRewind;
		Server_ActiveRewind(PositionRewind, RotationRewind);
	}

	bWantsToRewind = true;
}

bool UShooterCharacterMovement::Server_WallJump_Validate(const FVector & NewWallJumpDirection)
{
	return true;
}

void UShooterCharacterMovement::Server_WallJump_Implementation(const FVector & NewWallJumpDirection)
{
	WallJumpDirection = NewWallJumpDirection;
}

void UShooterCharacterMovement::WallJump(FVector & NewWallJumpDirection)
{
	if (PawnOwner->IsLocallyControlled()) {
		WallJumpDirection = NewWallJumpDirection;
		Server_WallJump(WallJumpDirection);
	}

	bWantsToWallJump = true;
}



