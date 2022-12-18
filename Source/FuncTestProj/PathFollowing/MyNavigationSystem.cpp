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
			if (InQuery.NavData.IsValid() == true)
			{
				const FColor DebugColor = FColor::White;
				const bool bPersistent = false;

				// draw path
				Result.Path->DebugDraw(InQuery.NavData.Get(), DebugColor, nullptr, bPersistent, m_DrawDebugLifeTime);
			}
		}
	}

	return Result;
}

void UMyNavigationSystem::DrawDebugPath(bool InFlag, float InLifeTime)
{
	m_DrawDebugPath = InFlag;
	m_DrawDebugLifeTime = InLifeTime;
}