// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FuncTestProj/System/MyObject.h"

#include "NavigationSystem.h"

#include "MyNavigationSystem.generated.h"

/**
 * 
 */
UCLASS()
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

	// Debug
	void DrawDebugPath(bool InFlag, float InLifeTime);
	void DrawPath(FNavPathSharedPtr InPath);

public:
	static bool m_DrawDebugPath;
	static float m_DrawDebugLifeTime;
};
