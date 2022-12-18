// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacterMovementComponent.generated.h"

/**
 * 
 */

struct FMyForceMoveData
{
public:
	FMyForceMoveData()
		: Direction(FVector::ZeroVector)
		, Power(0.0f)
		, IsProcessing(false)
	{

	}

	FMyForceMoveData(const FVector& InDir, const float InPower)
		: Direction(InDir)
		, Power(InPower)
		, IsProcessing(true)
	{

	}

public:
	FVector Direction;
	float Power;
	bool IsProcessing;
};

UCLASS()
class FUNCTESTPROJ_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UMyCharacterMovementComponent();

	virtual void CalcVelocity(float InDeltaTime, float InFriction, bool bFluid, float InBrakingDeceleration) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (AllowPrivateAccess = "true", DisplayName = "Use Max Friction"))
		bool m_UseMaxFriction;
};
