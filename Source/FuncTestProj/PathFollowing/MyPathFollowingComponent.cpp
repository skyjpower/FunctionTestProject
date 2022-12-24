// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPathFollowingComponent.h"

#include "FuncTestProj/PathFollowing/MyNavigationSystem.h"

#include "DrawDebugHelpers.h"

UMyPathFollowingComponent::UMyPathFollowingComponent()
	: m_DebugSplinePath(nullptr)
{

}

void UMyPathFollowingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UMyNavigationSystem::m_DrawDebugSplinePath == true)
	{
		if (m_DebugSplinePath != nullptr)
		{
			/*const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
			float&& SplineInputKey = m_DebugSplinePath->FindInputKeyClosestToWorldLocation(CurrentLocation);

			FVector SegmentLoc = m_DebugSplinePath->GetLocationAtSplineInputKey(SplineInputKey, ESplineCoordinateSpace::World);
			FVector SegmentDir = m_DebugSplinePath->GetDirectionAtSplineInputKey(SplineInputKey, ESplineCoordinateSpace::World);
			DrawDebugDirectionalArrow(GetWorld(), SegmentLoc, SegmentLoc + SegmentDir * 50.0f, 20.0f, FColor::Green, false, DeltaTime);
			DrawDebugSphere(GetWorld(), SegmentLoc, 5.0f, 20, FColor::Red);*/

			/*int32&& SplinePointNum = m_DebugSplinePath->GetNumberOfSplinePoints();
			for (int32 SplinePtIdx = 0; SplinePtIdx < SplinePointNum; ++SplinePtIdx)
			{
				FVector Loc = m_DebugSplinePath->GetLocationAtSplineInputKey(static_cast<float>(SplinePtIdx), ESplineCoordinateSpace::World);
				FVector Dir = m_DebugSplinePath->GetDirectionAtSplineInputKey(static_cast<float>(SplinePtIdx), ESplineCoordinateSpace::World);
				DrawDebugDirectionalArrow(GetWorld(), Loc, Loc + Dir * 50.0f, 20.0f, FColor::Red, false, DeltaTime);
				DrawDebugSphere(GetWorld(), Loc, 20.0f, 20, FColor::Red);

				if (SplinePtIdx < SplinePointNum - 1)
				{
					const float Interval = 0.2f;
					for (int32 i = 0; i < 5; ++i)
					{
						float&& SplineInpuyKey = (float)(i * Interval) + SplinePtIdx;

						FVector SegmentLoc = m_DebugSplinePath->GetLocationAtSplineInputKey(SplineInpuyKey, ESplineCoordinateSpace::World);
						FVector SegmentDir = m_DebugSplinePath->GetDirectionAtSplineInputKey(SplineInpuyKey, ESplineCoordinateSpace::World);
						DrawDebugDirectionalArrow(GetWorld(), SegmentLoc, SegmentLoc + SegmentDir * 50.0f, 20.0f, FColor::Green, false, DeltaTime);
						DrawDebugSphere(GetWorld(), SegmentLoc, 20.0f, 20, FColor::Green);
					}
				}
			}*/
		}
	}
}

void UMyPathFollowingComponent::FollowPathSegment(float InDeltaTime)
{
	if (!Path.IsValid() || MovementComp == nullptr)
	{
		return;
	}

	if (m_SplinePathParams.IsValid() == true)
	{
		FollowPathSegment_Spline(InDeltaTime);
	}

	else
	{
		FollowPathSegment_Normal(InDeltaTime);
	}
}

void UMyPathFollowingComponent::FollowPathSegment_Normal(float InDeltaTime)
{
	const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
	const FVector CurrentTarget = GetCurrentTargetLocation();

	// set to false by default, we will set set this back to true if appropriate
	bIsDecelerating = false;

	const bool bAccelerationBased = MovementComp->UseAccelerationForPathFollowing();
	if (bAccelerationBased)
	{
		CurrentMoveInput = (CurrentTarget - CurrentLocation).GetSafeNormal();

		if (bStopMovementOnFinish && (MoveSegmentStartIndex >= DecelerationSegmentIndex))
		{
			const FVector PathEnd = Path->GetEndLocation();
			const float DistToEndSq = FVector::DistSquared(CurrentLocation, PathEnd);
			const bool bShouldDecelerate = DistToEndSq < FMath::Square(CachedBrakingDistance);
			if (bShouldDecelerate)
			{
				bIsDecelerating = true;

				const float SpeedPct = FMath::Clamp(FMath::Sqrt(DistToEndSq) / CachedBrakingDistance, 0.0f, 1.0f);
				CurrentMoveInput *= SpeedPct;
			}
		}

		PostProcessMove.ExecuteIfBound(this, CurrentMoveInput);
		MovementComp->RequestPathMove(CurrentMoveInput);
	}

	else
	{
		FVector MoveVelocity = (CurrentTarget - CurrentLocation) / InDeltaTime;

		const int32 LastSegmentStartIndex = Path->GetPathPoints().Num() - 2;
		const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);

		PostProcessMove.ExecuteIfBound(this, MoveVelocity);
		MovementComp->RequestDirectMove(MoveVelocity, bNotFollowingLastSegment);
	}
}

void UMyPathFollowingComponent::FollowPathSegment_Spline(float InDeltaTime)
{
	const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
	const FVector CurrentTarget = GetCurrentTargetLocation();
	
	TObjectPtr<AActor> Owner = MovementComp->GetOwner();
	if (Owner == nullptr || Owner->IsValidLowLevel() == false)
	{
		return;
	}

	// set to false by default, we will set set this back to true if appropriate
	bIsDecelerating = false;

	const bool bAccelerationBased = MovementComp->UseAccelerationForPathFollowing();
	if (bAccelerationBased)
	{
		// ĳ���� ���� ����
		FVector&& ForwardVec = Owner->GetActorForwardVector();

		// Spline ���� Direction ��������
		float&& SplineInputKey = m_SplinePathParams.Path->FindInputKeyClosestToWorldLocation(CurrentLocation);
		FVector SegmentDir = m_SplinePathParams.Path->GetDirectionAtSplineInputKey(SplineInputKey, ESplineCoordinateSpace::World);

		// �������� ���� ����
		FVector&& ToDestDir = (CurrentTarget - CurrentLocation).GetSafeNormal();
		
		// � ��ǲ + ���� ��ǲ
		const float UsingDestInputRatio = 0.5f;
		FVector&& SegmentDir_ToDestDir = (SegmentDir + ToDestDir * UsingDestInputRatio).GetSafeNormal();

		{
			// � ��ǲ
			// CurrentMoveInput = SegmentDir;

			// ���� ��ǲ
			// CurrentMoveInput = ToDestDir;
			
			// � ��ǲ + ������ ��ǲ
			CurrentMoveInput = SegmentDir_ToDestDir;
		}

		// ĳ���� ������� ����
		{
			// �ʴ� ������ ����
			if (InDeltaTime > 0.0f)
			{
				const float InterpSpeed = 10.0f;
				CurrentMoveInput = FMath::VInterpTo(ForwardVec, CurrentMoveInput, InDeltaTime, InterpSpeed);
			}
		}

		if (UMyNavigationSystem::m_DrawDebugSplinePath == true)
		{
			const float ArrowSize = 100.0f;
			DrawDebugDirectionalArrow(GetWorld(), CurrentLocation, CurrentLocation + CurrentMoveInput * 100.0f, ArrowSize, FColor::Cyan, false, InDeltaTime);
			DrawDebugDirectionalArrow(GetWorld(), CurrentLocation, CurrentLocation + ForwardVec * 100.0f, ArrowSize, FColor::Red, false, InDeltaTime);
			DrawDebugDirectionalArrow(GetWorld(), CurrentLocation, CurrentLocation + SegmentDir * 100.0f, ArrowSize, FColor::Green, false, InDeltaTime);

		}

		if (bStopMovementOnFinish && (MoveSegmentStartIndex >= DecelerationSegmentIndex))
		{
			const FVector PathEnd = Path->GetEndLocation();
			const float DistToEndSq = FVector::DistSquared(CurrentLocation, PathEnd);
			const bool bShouldDecelerate = DistToEndSq < FMath::Square(CachedBrakingDistance);
			if (bShouldDecelerate)
			{
				bIsDecelerating = true;

				const float SpeedPct = FMath::Clamp(FMath::Sqrt(DistToEndSq) / CachedBrakingDistance, 0.0f, 1.0f);
				CurrentMoveInput *= SpeedPct;
			}
		}

		PostProcessMove.ExecuteIfBound(this, CurrentMoveInput);
		MovementComp->RequestPathMove(CurrentMoveInput);
	}

	else
	{
		FVector MoveVelocity = (CurrentTarget - CurrentLocation) / InDeltaTime;

		const int32 LastSegmentStartIndex = Path->GetPathPoints().Num() - 2;
		const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);

		PostProcessMove.ExecuteIfBound(this, MoveVelocity);
		MovementComp->RequestDirectMove(MoveVelocity, bNotFollowingLastSegment);
	}
}

void UMyPathFollowingComponent::Reset()
{
	Super::Reset();
	
	m_SplinePathParams.Reset();

	if (m_DebugSplinePath != nullptr && m_DebugSplinePath->IsValidLowLevel() == true)
	{
		m_DebugSplinePath->DestroyComponent();
	}
	m_DebugSplinePath = nullptr;
}

FAIRequestID UMyPathFollowingComponent::RequestSplineMove(const FAIMoveRequest& InRequestData, FNavPathSharedPtr InPath, const FSplinePathParams& InSplinePathParams)
{
	FAIRequestID RequestID = Super::RequestMove(InRequestData, InPath);
	if (RequestID != FAIRequestID::InvalidRequest)
	{
		m_SplinePathParams = InSplinePathParams;
		if (m_SplinePathParams.Path != nullptr)
		{
			// debug draw path ?
			if (UMyNavigationSystem::m_DrawDebugSplinePath == true)
			{
				if (CreateDebugSplinePath() == true)
				{
					m_DebugSplinePath->SetComponentToWorld(InSplinePathParams.Path->Transform);
					m_DebugSplinePath->SplineCurves = InSplinePathParams.Path->Path;
					// m_DebugSplinePath->UpdateSpline();
				}
			}
		}
	}
	return RequestID;
}

// ==== private
bool UMyPathFollowingComponent::CreateDebugSplinePath()
{
	if (m_DebugSplinePath != nullptr)
	{
		m_DebugSplinePath->DestroyComponent();
		m_DebugSplinePath = nullptr;
	}

	m_DebugSplinePath = NewObject<USplineComponent>(this);
	m_DebugSplinePath->RegisterComponent();
	m_DebugSplinePath->SetDrawDebug(true);
	return m_DebugSplinePath != nullptr;
}