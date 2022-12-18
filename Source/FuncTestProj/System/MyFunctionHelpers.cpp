// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFunctionHelpers.h"

#include "FuncTestProj/System/MyGameInstance.h"

#include "FuncTestProj/System/MyObject.h"

TObjectPtr<UMyGameInstance> UMyFunctionHelpers::GetGameInstance(TObjectPtr<UObject> InWorldContext)
{
	TObjectPtr<UWorld> World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull);
	return World ? World->GetGameInstance<UMyGameInstance>() : nullptr;
}

TObjectPtr<UMyObject> UMyFunctionHelpers::GetInstance(TObjectPtr<UObject> InWorldContext, TSubclassOf<UMyObject> InInstanceClass)
{
	if (InWorldContext == nullptr || InInstanceClass == nullptr)
	{
		return nullptr;
	}

	TObjectPtr<UMyGameInstance> GameInstance = GetGameInstance(InWorldContext);
	if (GameInstance == nullptr)
	{
		return nullptr;
	}

	return GameInstance->GetInstance(InInstanceClass->GetFName());
}