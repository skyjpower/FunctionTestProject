// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FuncTestProj/System/MyObject.h"

#include "MyConsoleSystem.generated.h"

/**
 * 
 */
UCLASS()
class FUNCTESTPROJ_API UMyConsoleSystem : public UMyObject
{
	GENERATED_BODY()
	
public:
	UMyConsoleSystem();

	//~ UMyObject interface begin
	virtual void Start() override;
	virtual void Release() override;
	//~ UMyObject interface end
};
