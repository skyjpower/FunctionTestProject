// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Navigation/PathFollowingComponent.h"

#include "FuncTestProj/PathFollowing/MyNavigationSystem.h"

#include "MyPathFollowingComponent.generated.h"


struct FSplinePathParams
{
public:
	FSplinePathParams()
		: Path(nullptr)
		, IgnoreSplineHeight(false)
	{

	}

	FSplinePathParams(FSplinePathSharedPtr InPath, bool bIgnoreSplineHeight)
		: Path(InPath)
		, IgnoreSplineHeight(bIgnoreSplineHeight)
	{

	}

	bool IsValid() const
	{
		return Path != nullptr && Path->IsValid() == true;
	}

	void Reset()
	{
		Path = nullptr;
		IgnoreSplineHeight = false;
	}

public:
	FSplinePathSharedPtr Path;
	bool IgnoreSplineHeight;
};

UCLASS()
class FUNCTESTPROJ_API UMyPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()
	
public:
	UMyPathFollowingComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void FollowPathSegment(float InDeltaTime) override;
	virtual void Reset() override;

	virtual FAIRequestID RequestSplineMove(const FAIMoveRequest& InRequestData, FNavPathSharedPtr InPath, const FSplinePathParams& InSplinePathParams);

private:
	bool CreateDebugSplinePath();

	/*
		일반 이동 함수
	*/
	void FollowPathSegment_Normal(float InDeltaTime);
	/*
		스플라인 전용 이동 함수
	*/
	void FollowPathSegment_Spline(float InDeltaTime);

private:
	FSplinePathParams m_SplinePathParams;

	// 디버깅용
	UPROPERTY(EditAnywhere, Transient)
		TObjectPtr<USplineComponent> m_DebugSplinePath;
};
