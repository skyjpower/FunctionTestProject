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
		Path ��ȿ�� �˻�
	*/
	bool IsValid() const;

	/*
		@InPosition			: ����Ʈ�� ��ġ
		@InCoordinateSpace	: ���� / ���� ��ǥ�� ����
		@EInterpCurveMode	: ����� Spline Type
	*/
	void AddSplinePoint(const FVector& InPosition, ESplineCoordinateSpace::Type InCoordinateSpace, ESplinePointType::Type InCurveMode);

	/*
		FSplineCurves::UpdateSpline() �� Default Option���� ȣ���մϴ�.
	*/
	void UpdateSpline();
	
	/*
		�־��� ���� ��ġ(InWorldLocation)�κ��� ���� ����� Spline InputKey ����
	*/
	float FindInputKeyClosestToWorldLocation(const FVector& InWorldLocation) const;
	/*
		�־��� Spline InputKey�� �ش��ϴ� Direction ����
	*/
	FVector GetDirectionAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;
	/*
		�־��� Spline InputKey�� �ش��ϴ� Location ����
	*/
	FVector GetLocationAtSplineInputKey(float InKey, ESplineCoordinateSpace::Type CoordinateSpace) const;
	/*
		�־��� ���� ��ġ(InWorldLocation)�κ��� ���� ����� Spline Distance ����
	*/
	float GetDistanceClosestToWorldLocation(const FVector& InWorldLocation) const;
	/*
		�־��� ��ǲ Ű�� �˸��� Spline Distance ����
	*/
	float GetDistanceAlongSplineAtSplineInputKey(float InKey) const;

	/** Returns the length of the specified spline segment up to the parametric value given */
	float GetSegmentLength(const int32 Index, const float Param = 1.0f) const;

public:
	// �߽��� �� Transform
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

	// >> : �׽�Ʈ �Լ�
	void Test_BuildSplinePath(FNavPathSharedPtr InPath, FSplinePathSharedPtr OutSplinePath);
	// << :

public:
	/*
		spline path�� ������ �� ����� Interp ���
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path")
		TEnumAsByte<ESplinePointType::Type> m_SplinePathInterpType;

	/*
		������ spline path�� LeaveTagent�� size�� clamp �� �� �ɼ�
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Clamped LeaveTangent")
		bool m_UseClampedLeaveTangent;

	/*
		������ spline path�� ArriveTagent�� size�� clamp �� �� �ɼ�
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Clamped ArriveTangent")
		bool m_UseClampedArriveTangent;

	/*
		������ spline path�� ����� ����Ʈ�鿡 ���� ������ �� ����
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Merge Close Points")
		bool m_UseMergeClosePoints;

	/*
		spline path�� ����Ʈ���� ���� ���� �Ǵ� �Ÿ�
	*/
	UPROPERTY(config, EditDefaultsOnly, Category = "Spline Path", DisplayName = "Use Merge Close Points", meta = (EditCondition = "m_UseMergeClosePoints == true"))
		float m_MergeDistance;

	static bool m_DrawDebugPath;
	static bool m_DrawDebugSplinePath;
	static float m_DrawDebugLifeTime;
};
