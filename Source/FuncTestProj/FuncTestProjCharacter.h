// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"

#include "FuncTestProjCharacter.generated.h"

class UMyPathFollowingComponent;
class URequestMoveComponent;

UCLASS(config=Game)
class AFuncTestProjCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/*
		Custom Path Following Component
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Move, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UMyPathFollowingComponent> PathFollowingComponent;

	/*
		Request path following component
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Move, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<URequestMoveComponent> RequestMoveComponent;

public:
	AFuncTestProjCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

protected:

	/*
		Called for forwards/backward input 
		: 유저의 인풋이 들어왔을 때 PathFollowing을 종료시킵니다.
	*/
	void MoveForward(float Value);

	/*
		Called for side to side input
		: 유저의 인풋이 들어왔을 때 PathFollowing을 종료시킵니다.
	*/
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

