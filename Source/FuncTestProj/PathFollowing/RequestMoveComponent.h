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

/*
	�񵿱� �� ã�� ��û ĳ�̿�
*/
struct FRequestAsyncMoveParams
{
public:
	FRequestAsyncMoveParams();

	bool IsValid() const;
	void Clear();

public:
	uint32 QueryID;
	FAIMoveRequest MoveRequest;
};

/*
	�񵿱� �� ã�� ��û
*/
struct FRequestAsyncMoveResult
{
public:
	FRequestAsyncMoveResult();

	/*
		QueryID	: not INVALID_NAVQUERYID
		Code	: EPathFollowingRequestResult::RequestSuccessful
	*/
	bool IsRequestSuccessful() const;
	/*
		QueryID	: INVALID_NAVQUERYID
		Code	: EPathFollowingRequestResult::AlreadyAtGoal
	*/
	bool IsAlreadyAtGoal() const;
	/*
		QueryID	: INVALID_NAVQUERYID
		Code	: EPathFollowingRequestResult::Fail
	*/
	bool IsFailure() const;

	// debug
	FString ToString() const;

public:
	uint32 QueryID;
	EPathFollowingRequestResult::Type Code;
	FAIRequestID MoveId;
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
		* �̵� ��û�� �ϱ����� �ݵ�� UMyPathFollowingComponent�� ���õǾ�� �մϴ�.
		�⺻������ OnRegister���� Owner�κ��� UMyPathFollowingComponent�� Ž���մϴ�.
		@ref URequestMoveComponent::OnRegister()
	*/
	void SetPathFollowingComponent(TObjectPtr<UMyPathFollowingComponent> InPathFollowingComponent);
	bool HasPathFollowingComponent() const;

#pragma region request move interface

	/*
		UMyPathFollowingComponent::GetStatus() != EPathFollowingStatus::Idle 
	*/
	bool IsPathFollowing() const;

	/*
		Blueprint
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move To Location", ScriptName = "MoveToLocation"))
		EPathFollowingRequestResult::Type K2_MoveToLocation(const FVector& InDestination, FRequestMoveParams InMoveParams);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move To Actor", ScriptName = "MoveToActor"))
		EPathFollowingRequestResult::Type K2_MoveToActor(AActor* InGoal, FRequestMoveParams InMoveParams);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move To Location Async", ScriptName = "MoveToLocationAsync"))
		EPathFollowingRequestResult::Type K2_MoveToLocationAsync(const FVector& InDestination, FRequestMoveParams InMoveParams);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move To Actor Async", ScriptName = "MoveToActorAsync"))
		EPathFollowingRequestResult::Type K2_MoveToActorAsync(AActor* InGoal, FRequestMoveParams InMoveParams);

	/*
		Request Move Interfaces
	*/
	EPathFollowingRequestResult::Type MoveToLocation(const FVector& InDestination, const FRequestMoveParams& InMoveParams);
	EPathFollowingRequestResult::Type MoveToActor(AActor* InGoal, const FRequestMoveParams& InMoveParams);
	FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr);

	/*
		Request Move Async Interfaces
	*/
	FRequestAsyncMoveResult MoveToLocationAsync(const FVector& InDestination, const FRequestMoveParams& InMoveParams);
	FRequestAsyncMoveResult MoveToActorAsync(AActor* InGoal, const FRequestMoveParams& InMoveParams);
	FRequestAsyncMoveResult MoveToAsync(const FAIMoveRequest& MoveRequest);

	/*
		Stop Movement Interfaces
	*/
	void StopMovement(FAIRequestID InRequestID, EPathFollowingVelocityMode InVelocityMode = EPathFollowingVelocityMode::Keep);
	void StopCurrentMovement(EPathFollowingVelocityMode InVelocityMode = EPathFollowingVelocityMode::Keep);
	void AbortAsyncNavQuery();

#pragma endregion // request move interface

	bool ProcessCanRequestMove(const FAIMoveRequest& InMoveRequest);
	bool CheckAlreadyAtGoal(const FAIMoveRequest& InMoveRequest);

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

private:
	/*
		�̵��� ��û�ϱ� ���� �ؾ��� �۾� ����
	*/
	void ProcessBeforeRequestMove();

	/*
		FPathFindingQuery ����
	*/
	bool BuildPathfindingQuery(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& OutQuery) const;

	/*
		�� ã�� ����
	*/
	void FindPathForMoveRequest(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& InQuery, FNavPathSharedPtr& OutPath) const;

	/*
		�񵿱� �� ã�� �Ϸ� �ݹ�
	*/
	void OnPathFindingAsync(uint32 InQueryID, ENavigationQueryResult::Type InResult, FNavPathSharedPtr InPath);

	TObjectPtr<APawn> GetPawn() const;
	const FNavAgentProperties& GetNavAgentPropertiesRef() const;
	FVector GetNavAgentLocation() const;

private:
	TWeakObjectPtr<UMyPathFollowingComponent> m_CachePathFollowingComponent;

	// �񵿱� �� ã�� ������ ĳ��
	FRequestAsyncMoveParams m_AsyncMoveParams;
};
