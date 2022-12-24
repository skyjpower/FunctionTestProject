// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FuncTestProj/System/MyObject.h"

#include "NavigationSystem.h"

#include "Components/SplineComponent.h"

#include "MyNavigationSystem.generated.h"

struct FUNCTESTPROJ_API FSplinePath
{
public:
	FSplinePath();
	~FSplinePath();

public:
	/*
		Path 유효성 검사
	*/
	bool IsValid() const;

	/*
		@InPosition			: 포인트의 위치
		@InCoordinateSpace	: 로컬 / 월드 좌표계 지정
		@EInterpCurveMode	: 사용할 Spline Type
	*/
	void AddSplinePoint(const FVector& InPosition, ESplineCoordinateSpace::Type InCoordinateSpace, ESplinePointType::Type InCurveMode);

	/*
		FSplineCurves::UpdateSpline() 를 Default Option으로 호출합니다.
	*/
	void UpdateSpline();
	
	/*
		주어진 월드 위치(InWorldLocation)로부터 가장 가까운 Spline InputKey 리턴
	*/
	float FindInputKeyClosestToWorldLocation(const FVector& InWorldLocation) const;
	/*
		주어진 Spline InputKey에 해당하는 Direction 리턴
	*/
	FVector GetDirectionAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;
	/*
		주어진 Spline InputKey에 해당하는 Location 리턴
	*/
	FVector GetLocationAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;
	/*
		주어진 월드 위치(InWorldLocation)로부터 가장 가까운 Spline Distance 리턴
	*/
	float GetDistanceClosestToWorldLocation(const FVector& InWorldLocation) const;
	/*
		주어진 인풋 키에 알맞은 Spline Distance 리턴
	*/
	float GetDistanceAlongSplineAtSplineInputKey(float InKey) const;

	/** Returns the length of the specified spline segment up to the parametric value given */
	float GetSegmentLength(const int32 Index, const float Param = 1.0f) const;

public:
	// 중심이 될 Transform
	FTransform Transform;

	// Path Data
	FSplineCurves Path;

	// debuging path id
private:
	uint32 m_ID;
	static uint32 s_UniqueID;

	// Spline path options
	static int32 s_ReparamStepsPerSegment;
};

typedef TSharedPtr<FSplinePath, ESPMode::ThreadSafe> FSplinePathSharedPtr;

struct FSplinePathFindingResult
{
public:
	FSplinePathFindingResult()
		: OriginPathResult(FPathFindingResult(ENavigationQueryResult::Error))
		, SplinePath(nullptr)
	{
	}

public:
	FPathFindingResult OriginPathResult;
	FSplinePathSharedPtr SplinePath;
};

UCLASS(config = Engine)
class FUNCTESTPROJ_API UMyNavigationSystem : public UMyObject
{
	GENERATED_BODY()
	
public:
	UMyNavigationSystem();

	//~ UMyObject interface begin
	virtual void Start() override;
	virtual void Tick(float InDeltaTime) override;
	virtual void Release() override;
	//~ UMyObject interface end

	TObjectPtr<UNavigationSystemV1> GetNavigationSystem() const;

	// UNavigationSystemV1 interfaces
	FPathFindingResult FindPathSync(FPathFindingQuery InQuery, EPathFindingMode::Type InMode = EPathFindingMode::Regular);
	uint32 FindPathAsync(const FNavAgentProperties& AgentProperties, FPathFindingQuery Query, const FNavPathQueryDelegate& ResultDelegate, EPathFindingMode::Type Mode = EPathFindingMode::Regular);
	void AbortAsyncFindPathRequest(const uint32 InAsynPathQueryID);

	// spline path interface
	FSplinePathFindingResult FindSplinePathSync(FPathFindingQuery InQuery, EPathFindingMode::Type InMode = EPathFindingMode::Regular);
	bool BuildSplinePath(FNavPathSharedPtr InPath, FSplinePathSharedPtr OutSplinePath);
	FSplinePathSharedPtr CreateSplinePathSharedPtr() const;

	// Debug
	void DrawDebugPath(bool InFlag, float InLifeTime);
	void DrawPath(FNavPathSharedPtr InPath);

	// >> : 테스트 함수
	void Test_BuildSplinePath(FNavPathSharedPtr InPath, FSplinePathSharedPtr OutSplinePath);
	// << :

public:
	/*
		spline path를 구성할 때 사용할 Interp 모드
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path")
		TEnumAsByte<ESplinePointType::Type> m_SplinePathInterpType;

	/*
		구성된 spline path의 LeaveTagent의 size를 clamp 할 지 옵션
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Clamped LeaveTangent")
		bool m_UseClampedLeaveTangent;

	/*
		구성된 spline path의 ArriveTagent의 size를 clamp 할 지 옵션
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Clamped ArriveTangent")
		bool m_UseClampedArriveTangent;

	/*
		구성된 spline path의 가까운 포인트들에 대해 병합할 지 여부
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Merge Close Points")
		bool m_UseMergeClosePoints;

	/*
		spline path의 포인트들의 병합 여부 판단 거리
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Merge Close Points", meta = (EditCondition = "m_UseMergeClosePoints == true"))
		float m_MergeDistance;

	static bool m_DrawDebugPath;
	static bool m_DrawDebugSplinePath;
	static float m_DrawDebugLifeTime;
};
