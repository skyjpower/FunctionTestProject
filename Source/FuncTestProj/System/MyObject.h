// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyObject.generated.h"

/**
 * 
 */
UCLASS()
class FUNCTESTPROJ_API UMyObject : public UObject
{
	GENERATED_BODY()
	
public:
	UMyObject();

	virtual void Start();
	virtual void Tick(float InDeltaTime);
	virtual void Release();
};
