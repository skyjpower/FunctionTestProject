// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"

#include "Tickable.h"

#include "MyGameInstance.generated.h"

class UMyObject;

UCLASS()
class FUNCTESTPROJ_API UMyGameInstance : public UGameInstance, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UMyGameInstance();

	// >> : UGameInstance interface
	virtual void Init() override;
	virtual void Shutdown() override;
	// << : UGameInstance interface

	// >> : FTickableGameObject interface
	virtual void Tick(float InDeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	// << : FTickableGameObject interface

	TObjectPtr<UMyObject> GetInstance(const FName& InKey) const;
	template<typename T>
	TObjectPtr<T> GetInstance()
	{
		TObjectPtr<UMyObject> Instance = GetInstance(T::StaticClass()->GetFName());
		if (Instance == nullptr)
		{
			return nullptr;
		}
		return Cast<T>(Instance);
	}

private:
	void AddInstance(TSubclassOf<UMyObject> InInstanceClass);

	UPROPERTY(Transient)
		TArray<TObjectPtr<UMyObject>> m_InstanceList;

	// Instance 접근을 위한 맵
	UPROPERTY(Transient)
		TMap<FName, TObjectPtr<UMyObject>> m_InstanceMap;
};
