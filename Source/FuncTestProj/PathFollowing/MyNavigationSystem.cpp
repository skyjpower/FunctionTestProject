// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNavigationSystem.h"

bool UMyNavigationSystem::m_DrawDebugPath = false;
float UMyNavigationSystem::m_DrawDebugLifeTime = 10.0f;

UMyNavigationSystem::UMyNavigationSystem()
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