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
			/*int32&& SplinePointNum = m_DebugSplinePath->GetNumberOfSplinePoints();
			for (int32 SplinePtIdx = 0; SplinePtIdx < SplinePointNum; ++SplinePtIdx)
			{
				FVector&& Loc = m_DebugSplinePath->GetLocationAtSplinePoint(SplinePtIdx, ESplineCoordinateSpace::World);

				FVector&& LeaveDir = m_DebugSplinePath->GetTangentAtSplinePoint(SplinePtIdx, ESplineCoordinateSpace::World);
				DrawDebugDirectionalArrow(GetWorld(), Loc, Loc + LeaveDir, 20.0f, FColor::Red, false, DeltaTime);
				
				FVector&& ArriveDir = m_DebugSplinePath->GetArriveTangentAtSplinePoint(SplinePtIdx, ESplineCoordinateSpace::World);
				DrawDebugDirectionalArrow(GetWorld(), Loc, Loc + ArriveDir, 20.0f, FColor::Green, false, DeltaTime);
			}*/
		}
	}
}

void UMyPathFollowingComponent::FollowPathSegment(float InDeltaTime)
{
	Super::FollowPathSegment(InDeltaTime);
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