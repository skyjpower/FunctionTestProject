// Fill out your copyright notice in the Description page of Project Settings.


#include "MyConsoleSystem.h"

#include "HAL/IConsoleManager.h"

#include "FuncTestProj/System/MyFunctionHelpers.h"

#include "FuncTestProj/PathFollowing/MyNavigationSystem.h"

namespace FMyConsole
{
	FAutoConsoleVariableRef CVars_MyNavigationSystem[] = {
			FAutoConsoleVariableRef(TEXT("MyNav.DrawPaths"), UMyNavigationSystem::m_DrawDebugPath, TEXT("Draw Paths"), ECVF_Default)
	};
}

UMyConsoleSystem::UMyConsoleSystem()
{
	
}

void UMyConsoleSystem::Start()
{
	Super::Start();
}

void UMyConsoleSystem::Release()
{
	Super::Release();
}