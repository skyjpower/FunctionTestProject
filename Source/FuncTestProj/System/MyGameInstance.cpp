// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameInstance.h"

#include "FuncTestProj/PathFollowing/MyNavigationSystem.h"
#include "FuncTestProj/System/MyConsoleSystem.h"

UMyGameInstance::UMyGameInstance()
{

}

void UMyGameInstance::Init()
{
	Super::Init();

	// add instance
	AddInstance(UMyNavigationSystem::StaticClass());
	AddInstance(UMyConsoleSystem::StaticClass());

	// call start
	for (TObjectPtr<UMyObject> Instance : m_InstanceList)
	{
		if (Instance == nullptr)
		{
			continue;
		}
		Instance->Start();
	}
}

void UMyGameInstance::Shutdown()
{
	// instance release
	for (TObjectPtr<UMyObject> Instance : m_InstanceList)
	{
		if (Instance == nullptr)
		{
			continue;
		}
		Instance->Release();
	}
	m_InstanceList.Empty(0);

	Super::Shutdown();
}

void UMyGameInstance::Tick(float InDeltaTime)
{
	// instance tick
	for (TObjectPtr<UMyObject> Instance : m_InstanceList)
	{
		if (Instance == nullptr)
		{
			continue;
		}
		Instance->Tick(InDeltaTime);
	}
}

bool UMyGameInstance::IsTickable() const
{
	return true;
}

TStatId UMyGameInstance::GetStatId() const
{
	return UObject::GetStatID();
}

TObjectPtr<UMyObject> UMyGameInstance::GetInstance(const FName& InKey) const
{
	TObjectPtr<UMyObject> const* FindInstance = m_InstanceMap.Find(InKey);
	if (FindInstance == nullptr)
	{
		return nullptr;
	}

	return *FindInstance;
}

// private ====
void UMyGameInstance::AddInstance(TSubclassOf<UMyObject> InInstanceClass)
{
	if (InInstanceClass == nullptr)
	{
		return;
	}

	FName&& Key = InInstanceClass->GetFName();
	if (m_InstanceMap.Contains(Key) == true)
	{
		return;
	}

	TObjectPtr<UMyObject> NewInstance = NewObject<UMyObject>(this, InInstanceClass, Key);
	if (NewInstance != nullptr)
	{
		m_InstanceList.Emplace(NewInstance);
		m_InstanceMap.Emplace(Key, NewInstance);
	}
}