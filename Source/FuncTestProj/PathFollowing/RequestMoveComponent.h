// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"

#include "FuncTestProj/PathFollowing/MyPathFollowingComponent.h"

#include "RequestMoveComponent.generated.h"

USTRUCT(BlueprintType)
struct FRequestMoveParams
{
	GENERATED_BODY()

public:
	FRequestMoveParams()
		: AcceptanceRadius(-1.0f)
		, StopOnOverlap(true)
		, UsePathfinding(true)
		, ProjectDestinationToNavigation(true)
		, CanStrafe(false)
		, FilterClass(nullptr)
		, AllowPartialPaths(false)
	{

	}

public:
	// �����ߴٰ� �Ǵ��� �Ÿ��� ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AcceptanceRadius;
	/*
		: Overlap �Ǿ��� �� �̵� �Ϸ� ó�� ����
		EPathFollowingReachMode, AActor::GetSimpleCollisionCylinder() ����
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool StopOnOverlap;

	/*
		�� ã�⸦ ������ �� ����
		: �������� �ʴ´ٸ� �������� ��ٷ� �̵��մϴ�.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool UsePathfinding;

	/*
		�������� Location�� ��� Nav �˻縦 �� ��
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ProjectDestinationToNavigation;

	/*
		������ ? �������� �˾ƺ�����
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool CanStrafe;

	/*
		�� ã�⿡ ������ ����� Filter class
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UNavigationQueryFilter> FilterClass;

	/*
		�κ��� �� ã�⸦ ����� ������
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool AllowPartialPaths;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FUNCTESTPROJ_API URequestMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URequestMoveComponent();

public:
	virtual void BeginPlay() override;

	/*
		�̵� ��û�� �ϱ����� �ݵ�� UMyPathFollowingComponent�� ���õǾ�� �մϴ�.
	*/
	void SetPathFollowingComponent(TObjectPtr<UMyPathFollowingComponent> InPathFollowingComponent);
	bool HasPathFollowingComponent() const;

#pragma region request move interface

	/*
		UMyPathFollowingComponent::GetStatus() != EPathFollowingStatus::Idle 
	*/
	bool IsPathFollowing() const;

	/*
		Blueprint��
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move To Location", ScriptName = "MoveToLocation"))
		EPathFollowingRequestResult::Type K2_MoveToLocation(const FVector& InDestination, FRequestMoveParams InMoveParams);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move To Actor", ScriptName = "MoveToActor"))
		EPathFollowingRequestResult::Type K2_MoveToActor(AActor* InGoal, FRequestMoveParams InMoveParams);

	/*
		�̵� ��û �������̽�
	*/
	EPathFollowingRequestResult::Type MoveToLocation(const FVector& InDestination, const FRequestMoveParams& InMoveParams);
	EPathFollowingRequestResult::Type MoveToActor(AActor* InGoal, const FRequestMoveParams& InMoveParams);
	FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr);

	/*
		�̵� ���� �������̽�
	*/
	void StopMovement(FAIRequestID InRequestID, EPathFollowingVelocityMode InVelocityMode = EPathFollowingVelocityMode::Keep);
	void StopCurrentMovement(EPathFollowingVelocityMode InVelocityMode = EPathFollowingVelocityMode::Keep);

#pragma endregion // request move interface

private:
	/*
		�̵��� ��û�ϱ� ���� �ؾ��� �۾� ����
	*/
	void ProcessBeforeRequestMove(bool InStopCurrentMovement);

	/*
		FPathFindingQuery ����
	*/
	bool BuildPathfindingQuery(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& InQuery) const;
	void FindPathForMoveRequest(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& InQuery, FNavPathSharedPtr& OutPath) const;

	TObjectPtr<APawn> GetPawn() const;
	const FNavAgentProperties& GetNavAgentPropertiesRef() const;
	FVector GetNavAgentLocation() const;

private:
	TWeakObjectPtr<UMyPathFollowingComponent> m_CachePathFollowingComponent;
};
