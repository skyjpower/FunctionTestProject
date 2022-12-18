// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "MyFunctionHelpers.generated.h"

class UMyGameInstance;
class UMyObject;

UCLASS()
class FUNCTESTPROJ_API UMyFunctionHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TObjectPtr<UMyGameInstance> GetGameInstance(TObjectPtr<UObject> InWorldContext);

	static TObjectPtr<UMyObject> GetInstance(TObjectPtr<UObject> InWorldContext, TSubclassOf<UMyObject> InInstanceClass);

	template<typename T>
	static T* GetInstance(TObjectPtr<UObject> InWorldContext)
	{
		return Cast<T>(GetInstance(InWorldContext, T::StaticClass()));
	}
};
