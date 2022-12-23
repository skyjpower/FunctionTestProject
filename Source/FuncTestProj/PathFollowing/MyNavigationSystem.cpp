// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNavigationSystem.h"

bool UMyNavigationSystem::m_DrawDebugPath = false;
bool UMyNavigationSystem::m_DrawDebugSplinePath = false;
float UMyNavigationSystem::m_DrawDebugLifeTime = 10.0f;

#pragma region FSplinePath
uint32 FSplinePath::s_UniqueID = uint32(0);

FSplinePath::FSplinePath()
	: Transform(FTransform::Identity)
	, m_ID(++s_UniqueID)
{
	UE_LOG(LogClass, Log, TEXT("Create Path, ID[%d]"), m_ID);
}

FSplinePath::~FSplinePath()
{
	UE_LOG(LogClass, Log, TEXT("Delete Path, ID[%d]"), m_ID);
}

void FSplinePath::AddSplinePoint(const FVector& InPosition, ESplineCoordinateSpace::Type InCoordinateSpace, ESplinePointType::Type InCurveMode)
{
	const FVector TransformedPosition = (InCoordinateSpace == ESplineCoordinateSpace::World) ?
		Transform.InverseTransformPosition(InPosition) : InPosition;

	// Add the spline point at the end of the array, adding 1.0 to the current last input key.
	// This continues the former behavior in which spline points had to be separated by an interval of 1.0.
	const float InKey = Path.Position.Points.Num() ? Path.Position.Points.Last().InVal + 1.0f : 0.0f;

	EInterpCurveMode&& InterpCurveMode = ConvertSplinePointTypeToInterpCurveMode(InCurveMode);

	Path.Position.Points.Emplace(InKey, TransformedPosition, FVector::ZeroVector, FVector::ZeroVector, InterpCurveMode);
	Path.Rotation.Points.Emplace(InKey, FQuat::Identity, FQuat::Identity, FQuat::Identity, InterpCurveMode);
	Path.Scale.Points.Emplace(InKey, FVector(1.0f), FVector::ZeroVector, FVector::ZeroVector, InterpCurveMode);
}

void FSplinePath::UpdateSpline()
{
	const bool bClosedLoop = false;
	const bool bStationaryEndpoints = false;
	int32 ReparamStepsPerSegment = 10;
	bool bLoopPositionOverride = false;
	float LoopPosition = 0.0f;
	const FVector& Scale3D = FVector(1.0f);

	// leave, arrive tangent가 세팅되도록 Update를 한 번 해줍니다.
	Path.UpdateSpline(bClosedLoop, bStationaryEndpoints, ReparamStepsPerSegment, bLoopPositionOverride, LoopPosition, Scale3D);
}
#pragma endregion


UMyNavigationSystem::UMyNavigationSystem()
	: m_SplinePathInterpType(ESplinePointType::Curve)
	, m_UseClampedLeaveTangent(false)
	, m_UseClampedArriveTangent(false)
	, m_UseMergeClosePoints(false)
	, m_MergeDistance(150.0f)
{

}

void UMyNavigationSystem::Start()
{
	Super::Start();
}

void UMyNavigationSystem::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

void UMyNavigationSystem::Release()
{
	Super::Release();
}

TObjectPtr<UNavigationSystemV1> UMyNavigationSystem::GetNavigationSystem() const
{
	return FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
}

FPathFindingResult UMyNavigationSystem::FindPathSync(FPathFindingQuery InQuery, EPathFindingMode::Type InMode)
{
	FPathFindingResult Result(ENavigationQueryResult::Error);

	TObjectPtr<UNavigationSystemV1> NavSys = GetNavigationSystem();
	if (NavSys == nullptr)
	{
		return Result;
	}

	Result = NavSys->FindPathSync(InQuery, InMode);

	if (m_DrawDebugPath == true)
	{
		if (Result.IsSuccessful() == true)
		{
			DrawPath(Result.Path);
		}
	}

	return Result;
}

uint32 UMyNavigationSystem::FindPathAsync(const FNavAgentProperties& AgentProperties, FPathFindingQuery Query, const FNavPathQueryDelegate& ResultDelegate, EPathFindingMode::Type Mode)
{
	uint32 AsyncID(INVALID_NAVQUERYID);

	TObjectPtr<UNavigationSystemV1> NavSys = GetNavigationSystem();
	if (NavSys != nullptr)
	{
		AsyncID = NavSys->FindPathAsync(AgentProperties, Query, ResultDelegate, Mode);
	}

	return AsyncID;
}

void UMyNavigationSystem::AbortAsyncFindPathRequest(const uint32 InAsynPathQueryID)
{
	TObjectPtr<UNavigationSystemV1> NavSys = GetNavigationSystem();
	if (NavSys != nullptr)
	{
		NavSys->AbortAsyncFindPathRequest(InAsynPathQueryID);
	}
}

FSplinePathFindingResult UMyNavigationSystem::FindSplinePathSync(FPathFindingQuery InQuery, EPathFindingMode::Type InMode)
{
	FSplinePathFindingResult Result;
	Result.OriginPathResult = FindPathSync(InQuery, InMode);

	if (Result.OriginPathResult.IsSuccessful() == true)
	{
		// build spline path
		Result.SplinePath = CreateSplinePathSharedPtr();
		if (BuildSplinePath(Result.OriginPathResult.Path, Result.SplinePath) == false)
		{
			Result.OriginPathResult = FPathFindingResult(ENavigationQueryResult::Error);
		}
	}

	return Result;
}

bool UMyNavigationSystem::BuildSplinePath(FNavPathSharedPtr InPath, FSplinePathSharedPtr OutSplinePath)
{
	if (InPath == nullptr)
	{
		return false;
	}

	if (OutSplinePath == nullptr)
	{
		return false;
	}

	// PathPoints가 없는 경우
	if (InPath->IsValid() == false)
	{
		return false;
	}

	const TArray<FNavPathPoint>& PathPoints = InPath->GetPathPoints();

	// Path의 첫 시작점을 Location으로 이용합니다.
	OutSplinePath->Transform.SetLocation(PathPoints[0].Location);

	// merge 방식
	if (m_UseMergeClosePoints == true)
	{
		int32 PrevPathPointIndex = 0;
		int32&& NumPoints = PathPoints.Num();
		TArray<FVector> SplinePointList;
		SplinePointList.Reserve(NumPoints);
		SplinePointList.Emplace(PathPoints[0].Location);

		float&& SquareMergeDistance = FMath::Square(m_MergeDistance);
		for (int32 PathPointIdx = 1; PathPointIdx < NumPoints; ++PathPointIdx)
		{
			FVector LastLocation = SplinePointList.Last();
			
			// need merge point ?
			FVector&& ToPathPoint = PathPoints[PathPointIdx].Location - LastLocation;

			float TestSquaredLength = ToPathPoint.SquaredLength();
			float TestDistanceSquard = FVector::DistSquared(PathPoints[PathPointIdx].Location, LastLocation);

			if (ToPathPoint.SquaredLength() <= SquareMergeDistance)
			{
				// 1개인 경우 병합이 아닌 포인트를 제거합니다.
				if (SplinePointList.Num() == 1)
				{
					continue;
				}

				// 두 포인트의 중간점에 병합
				SplinePointList.Last() = LastLocation + ToPathPoint * 0.5f;
			}

			else
			{
				SplinePointList.Emplace(PathPoints[PathPointIdx].Location);
			}
		}

		// insert spline point
		for (const FVector& PathPoint : SplinePointList)
		{
			OutSplinePath->AddSplinePoint(PathPoint, ESplineCoordinateSpace::World, m_SplinePathInterpType);
		}
	}

	else
	{
		for (const FNavPathPoint& PathPoint : PathPoints)
		{
			OutSplinePath->AddSplinePoint(PathPoint.Location, ESplineCoordinateSpace::World, m_SplinePathInterpType);
		}
	}

	OutSplinePath->UpdateSpline();

	if (m_UseClampedLeaveTangent == true || m_UseClampedArriveTangent == true)
	{
		FSplineCurves& Path = OutSplinePath->Path;
		int32&& NumPoints = Path.Position.Points.Num();
		for (int32 PointIdx = 0; PointIdx < NumPoints; ++PointIdx)
		{
			const FVector& CurrentPointLoc = Path.Position.Points[PointIdx].OutVal;

			// clamp leave tangent
			if (m_UseClampedLeaveTangent == true)
			{
				const int32 NextPointIndex = PointIdx + 1;
				if (NextPointIndex < NumPoints)
				{
					const FVector& NextPointLoc = Path.Position.Points[NextPointIndex].OutVal;
					const float DistToNext = FVector::Distance(NextPointLoc, CurrentPointLoc);
					Path.Position.Points[PointIdx].LeaveTangent = Path.Position.Points[PointIdx].LeaveTangent.GetClampedToMaxSize(DistToNext);
				}
			}

			if (m_UseClampedArriveTangent == true)
			{
				// clamp arrive tangent
				const int32 PrevPointIndex = PointIdx - 1;
				if (PrevPointIndex >= 0)
				{
					const FVector& PrevPointLoc = Path.Position.Points[PrevPointIndex].OutVal;
					const float DistToPrev = FVector::Distance(PrevPointLoc, CurrentPointLoc);
					Path.Position.Points[PointIdx].ArriveTangent = Path.Position.Points[PointIdx].ArriveTangent.GetClampedToMaxSize(DistToPrev);
				}
			}

			Path.Position.Points[PointIdx].InterpMode = CIM_CurveUser;
		}

		OutSplinePath->UpdateSpline();
	}

	return true;
}

FSplinePathSharedPtr UMyNavigationSystem::CreateSplinePathSharedPtr() const
{
	FSplinePathSharedPtr Path = MakeShareable(new FSplinePath());
	return Path;
}

void UMyNavigationSystem::DrawDebugPath(bool InFlag, float InLifeTime)
{
	m_DrawDebugPath = InFlag;
	m_DrawDebugLifeTime = InLifeTime;
}

void UMyNavigationSystem::DrawPath(FNavPathSharedPtr InPath)
{
	if (m_DrawDebugPath == true)
	{
		if (InPath != nullptr)
		{
			TObjectPtr<UNavigationSystemV1> NavSys = GetNavigationSystem();
			if (NavSys != nullptr)
			{
				ANavigationData* NavData = NavSys->GetDefaultNavDataInstance();
				if (NavData != nullptr)
				{
					const FColor DebugColor = FColor::White;
					const bool bPersistent = false;

					// draw path
					InPath->DebugDraw(NavData, DebugColor, nullptr, bPersistent, m_DrawDebugLifeTime);
				}
			}
		}
	}
}

/*
	FSplinePathSharedPtr	: FSplinePath의 SharedPtr typedef
*/ 
void UMyNavigationSystem::Test_BuildSplinePath(FNavPathSharedPtr InPath, FSplinePathSharedPtr OutSplinePath)
{
	// 길찾기 결과 Path Point 리스트
	const TArray<FNavPathPoint>& PathPoints = InPath->GetPathPoints();

	for (const FNavPathPoint& PathPoint : PathPoints)
	{
		/*	
			m_SplinePathInterpType : ESplinePointType::Type
			USplineComponent::AddSplinePoint() 함수와 동일한 기능
		*/
		OutSplinePath->AddSplinePoint(PathPoint.Location, ESplineCoordinateSpace::World, m_SplinePathInterpType);
	}

	// USplineComponent::UpdateSpline() 함수와 동일한 기능
	OutSplinePath->UpdateSpline();
}

