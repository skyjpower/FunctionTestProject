// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "Kismet/GameplayStatics.h"

#include "Engine/TargetPoint.h"

#include "FuncTestProj/FuncTestProjCharacter.h"
#include "FuncTestProj/PathFollowing/RequestMoveComponent.h"


void AMyPlayerController::Test_AllCharacterMoveToTargetPoint(bool InAsync)
{
	TObjectPtr<AActor> TargetPoint = UGameplayStatics::GetActorOfClass(GetWorld(), ATargetPoint::StaticClass());
	if (TargetPoint == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Fail to find TargetPoint"));
		return;
	}

	// ATargetPoint
	// AFuncTestProjCharacter
	TArray<TObjectPtr<AActor>> TestActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFuncTestProjCharacter::StaticClass(), TestActors);

	FRequestMoveParams MoveParams;
	for (TObjectPtr<AActor> TestActor : TestActors)
	{
		TObjectPtr<AFuncTestProjCharacter> CastFuncCharacter = Cast<AFuncTestProjCharacter>(TestActor);
		if (CastFuncCharacter == nullptr)
		{
			continue;
		}

		if (CastFuncCharacter == GetPawn())
		{
			continue;
		}

		if (CastFuncCharacter->Tags.Contains(TEXT("exception")) == true)
		{
			continue;
		}

		TObjectPtr<URequestMoveComponent> RequestMoveComponent = CastFuncCharacter->GetRequestMoveComponent();

		if (InAsync == false)
		{
			RequestMoveComponent->MoveToLocation(TargetPoint->GetActorLocation(), FRequestMoveParams());
		}

		else
		{
			RequestMoveComponent->MoveToLocationAsync(TargetPoint->GetActorLocation(), FRequestMoveParams());
		}
	}
}