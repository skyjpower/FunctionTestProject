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
	// 도착했다고 판단할 거리에 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AcceptanceRadius;
	/*
		: Overlap 되었을 때 이동 완료 처리 여부
		EPathFollowingReachMode, AActor::GetSimpleCollisionCylinder() 참고
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool StopOnOverlap;

	/*
		길 찾기를 수행할 지 여부
		: 수행하지 않는다면 직선으로 곧바로 이동합니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool UsePathfinding;

	/*
		목적지가 Location인 경우 Nav 검사를 할 지
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ProjectDestinationToNavigation;

	/*
		옆걸음 ? 무엇인지 알아봐야함
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool CanStrafe;

	/*
		길 찾기에 정보로 사용할 Filter class
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UNavigationQueryFilter> FilterClass;

	/*
		부분적 길 찾기를 허용할 것인지
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool AllowPartialPaths;
};

/*
	비동기 길 찾기 요청 캐싱용
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
	비동기 길 찾기 요청
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
		* 이동 요청을 하기전에 반드시 UMyPathFollowingComponent가 세팅되어야 합니다.
		기본적으로 OnRegister에서 Owner로부터 UMyPathFollowingComponent를 탐색합니다.
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
		이동을 요청하기 전에 해야할 작업 수행
	*/
	void ProcessBeforeRequestMove();

	/*
		FPathFindingQuery 세팅
	*/
	bool BuildPathfindingQuery(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& OutQuery) const;

	/*
		길 찾기 수행
	*/
	void FindPathForMoveRequest(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& InQuery, FNavPathSharedPtr& OutPath) const;

	/*
		비동기 길 찾기 완료 콜백
	*/
	void OnPathFindingAsync(uint32 InQueryID, ENavigationQueryResult::Type InResult, FNavPathSharedPtr InPath);

	TObjectPtr<APawn> GetPawn() const;
	const FNavAgentProperties& GetNavAgentPropertiesRef() const;
	FVector GetNavAgentLocation() const;

private:
	TWeakObjectPtr<UMyPathFollowingComponent> m_CachePathFollowingComponent;

	// 비동기 길 찾기 데이터 캐싱
	FRequestAsyncMoveParams m_AsyncMoveParams;
};
