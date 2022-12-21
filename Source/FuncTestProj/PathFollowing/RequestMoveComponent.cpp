// Fill out your copyright notice in the Description page of Project Settings.


#include "RequestMoveComponent.h"

#include "FuncTestProj/System/MyFunctionHelpers.h"
#include "FuncTestProj/PathFollowing/MyNavigationSystem.h"

#pragma region FRequestAsyncMoveParams
FRequestAsyncMoveParams::FRequestAsyncMoveParams()
	: QueryID(INVALID_NAVQUERYID)
{

}
bool FRequestAsyncMoveParams::IsValid() const
{
	return QueryID != INVALID_NAVQUERYID;
}

void FRequestAsyncMoveParams::Clear()
{
	QueryID = INVALID_NAVQUERYID;
	MoveRequest = FAIMoveRequest();
}
#pragma endregion

#pragma region FRequestAsyncMoveResult
FRequestAsyncMoveResult::FRequestAsyncMoveResult()
	: QueryID(INVALID_NAVQUERYID)
	, Code(EPathFollowingRequestResult::Failed)
	, MoveId(FAIRequestID::InvalidRequest)
{

}

bool FRequestAsyncMoveResult::IsRequestSuccessful() const
{
	return Code == EPathFollowingRequestResult::RequestSuccessful;
}

bool FRequestAsyncMoveResult::IsAlreadyAtGoal() const
{
	return Code == EPathFollowingRequestResult::AlreadyAtGoal;
}

bool FRequestAsyncMoveResult::IsFailure() const
{
	return Code == EPathFollowingRequestResult::Failed;
}

FString FRequestAsyncMoveResult::ToString() const
{
	FString CodeStr = TEXT("");
	if (IsRequestSuccessful() == true)
	{
		CodeStr = TEXT("RequestSuccessful");
	}

	else if (IsAlreadyAtGoal() == true)
	{
		CodeStr = TEXT("AlreadyAtGoal");
	}

	else
	{
		CodeStr = TEXT("Failed");
	}

	return FString::Printf(TEXT("QueryID[%d], RequestResult[%s], MoveId[%d]"), QueryID, *CodeStr, MoveId.GetID());
}

#pragma endregion


URequestMoveComponent::URequestMoveComponent()
	: m_CachePathFollowingComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void URequestMoveComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URequestMoveComponent::SetPathFollowingComponent(TObjectPtr<UMyPathFollowingComponent> InPathFollowingComponent)
{
	m_CachePathFollowingComponent = InPathFollowingComponent;
}

bool URequestMoveComponent::HasPathFollowingComponent() const
{
	return m_CachePathFollowingComponent.IsValid();
}

bool URequestMoveComponent::IsPathFollowing() const
{
	if (m_CachePathFollowingComponent.IsValid() == false)
	{
		return false;
	}
	
	EPathFollowingStatus::Type Status = m_CachePathFollowingComponent->GetStatus();
	return Status != EPathFollowingStatus::Idle;
}

EPathFollowingRequestResult::Type URequestMoveComponent::K2_MoveToLocation(const FVector& InDestination, FRequestMoveParams InMoveParams)
{
	return MoveToLocation(InDestination, InMoveParams);
}

EPathFollowingRequestResult::Type URequestMoveComponent::K2_MoveToActor(AActor* InGoal, FRequestMoveParams InMoveParams)
{
	return MoveToActor(InGoal, InMoveParams);
}

EPathFollowingRequestResult::Type URequestMoveComponent::K2_MoveToLocationAsync(const FVector& InDestination, FRequestMoveParams InMoveParams)
{
	FRequestAsyncMoveResult Result = MoveToLocationAsync(InDestination, InMoveParams);

	// UE_LOG(LogClass, Log, TEXT("URequestMoveComponent::MoveToLocationAsync[BP]() : %s"), *Result.ToString());

	return Result.Code;
}

EPathFollowingRequestResult::Type URequestMoveComponent::K2_MoveToActorAsync(AActor* InGoal, FRequestMoveParams InMoveParams)
{
	FRequestAsyncMoveResult Result = MoveToActorAsync(InGoal, InMoveParams);

	// UE_LOG(LogClass, Log, TEXT("URequestMoveComponent::MoveToLocationAsync[BP]() : %s"), *Result.ToString());

	return Result.Code;
}

EPathFollowingRequestResult::Type URequestMoveComponent::MoveToLocation(const FVector& InDestination, const FRequestMoveParams& InMoveParams)
{
	FAIMoveRequest MoveReq(InDestination);
	MoveReq.SetUsePathfinding(InMoveParams.UsePathfinding);
	MoveReq.SetAllowPartialPath(InMoveParams.AllowPartialPaths);
	MoveReq.SetProjectGoalLocation(InMoveParams.ProjectDestinationToNavigation);
	MoveReq.SetNavigationFilter(InMoveParams.FilterClass);
	MoveReq.SetAcceptanceRadius(InMoveParams.AcceptanceRadius);
	MoveReq.SetReachTestIncludesAgentRadius(InMoveParams.StopOnOverlap);
	MoveReq.SetCanStrafe(InMoveParams.CanStrafe);

	return MoveTo(MoveReq);
}

EPathFollowingRequestResult::Type URequestMoveComponent::MoveToActor(AActor* InGoal, const FRequestMoveParams& InMoveParams)
{
	FAIMoveRequest MoveReq(InGoal);
	MoveReq.SetUsePathfinding(InMoveParams.UsePathfinding);
	MoveReq.SetAllowPartialPath(InMoveParams.AllowPartialPaths);
	MoveReq.SetNavigationFilter(InMoveParams.FilterClass);
	MoveReq.SetAcceptanceRadius(InMoveParams.AcceptanceRadius);
	MoveReq.SetReachTestIncludesAgentRadius(InMoveParams.StopOnOverlap);
	MoveReq.SetCanStrafe(InMoveParams.CanStrafe);

	return MoveTo(MoveReq);
}

FPathFollowingRequestResult URequestMoveComponent::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
	ProcessBeforeRequestMove();

	FPathFollowingRequestResult ResultData;
	ResultData.Code = EPathFollowingRequestResult::Failed;

	if (MoveRequest.IsValid() == false)
	{
		return ResultData;
	}

	if (HasPathFollowingComponent() == false)
	{
		UE_LOG(LogClass, Error, TEXT("MoveTo request failed due missing PathFollowingComponent"));
		return ResultData;
	}

	UMyPathFollowingComponent* PathFollowingComponent = m_CachePathFollowingComponent.Get();

	TObjectPtr<APawn> Pawn = GetPawn();
	if (Pawn == nullptr || Pawn->IsValidLowLevel() == false)
	{
		UE_LOG(LogClass, Error, TEXT("Owner pawn[APawn] is invalid"));
		return ResultData;
	}

	bool bCanRequestMove = ProcessCanRequestMove(MoveRequest);
	bool bAlreadyAtGoal = false;
	if (bCanRequestMove == true)
	{
		bAlreadyAtGoal = CheckAlreadyAtGoal(MoveRequest);
	}

	if (bAlreadyAtGoal == true)
	{
		ResultData.MoveId = PathFollowingComponent->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		ResultData.Code = EPathFollowingRequestResult::AlreadyAtGoal;
	}

	else if (bCanRequestMove == true)
	{
		FPathFindingQuery PFQuery;

		const bool bValidQuery = BuildPathfindingQuery(MoveRequest, PFQuery);
		if (bValidQuery)
		{
			FNavPathSharedPtr Path;
			FindPathForMoveRequest(MoveRequest, PFQuery, Path);

			const FAIRequestID RequestID = Path.IsValid() ? PathFollowingComponent->RequestMove(MoveRequest, Path) : FAIRequestID::InvalidRequest;
			if (RequestID.IsValid())
			{
				// bAllowStrafe = MoveRequest.CanStrafe();
				ResultData.MoveId = RequestID;
				ResultData.Code = EPathFollowingRequestResult::RequestSuccessful;

				if (OutPath)
				{
					*OutPath = Path;
				}
			}
		}
	}

	if (ResultData.Code == EPathFollowingRequestResult::Failed)
	{
		ResultData.MoveId = PathFollowingComponent->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
	}

	return ResultData;
}

FRequestAsyncMoveResult URequestMoveComponent::MoveToLocationAsync(const FVector& InDestination, const FRequestMoveParams& InMoveParams)
{
	FAIMoveRequest MoveReq(InDestination);
	MoveReq.SetUsePathfinding(InMoveParams.UsePathfinding);
	MoveReq.SetAllowPartialPath(InMoveParams.AllowPartialPaths);
	MoveReq.SetProjectGoalLocation(InMoveParams.ProjectDestinationToNavigation);
	MoveReq.SetNavigationFilter(InMoveParams.FilterClass);
	MoveReq.SetAcceptanceRadius(InMoveParams.AcceptanceRadius);
	MoveReq.SetReachTestIncludesAgentRadius(InMoveParams.StopOnOverlap);
	MoveReq.SetCanStrafe(InMoveParams.CanStrafe);

	return MoveToAsync(MoveReq);
}

FRequestAsyncMoveResult URequestMoveComponent::MoveToActorAsync(AActor* InGoal, const FRequestMoveParams& InMoveParams)
{
	FAIMoveRequest MoveReq(InGoal);
	MoveReq.SetUsePathfinding(InMoveParams.UsePathfinding);
	MoveReq.SetAllowPartialPath(InMoveParams.AllowPartialPaths);
	MoveReq.SetNavigationFilter(InMoveParams.FilterClass);
	MoveReq.SetAcceptanceRadius(InMoveParams.AcceptanceRadius);
	MoveReq.SetReachTestIncludesAgentRadius(InMoveParams.StopOnOverlap);
	MoveReq.SetCanStrafe(InMoveParams.CanStrafe);

	return MoveToAsync(MoveReq);
}

FRequestAsyncMoveResult URequestMoveComponent::MoveToAsync(const FAIMoveRequest& MoveRequest)
{
	ProcessBeforeRequestMove();

	// 현재 수행 중인 비동기 Query ID 종료
	AbortAsyncNavQuery();

	FRequestAsyncMoveResult ResultData;
	ResultData.Code = EPathFollowingRequestResult::Failed;

	if (MoveRequest.IsValid() == false)
	{
		return ResultData;
	}

	if (HasPathFollowingComponent() == false)
	{
		UE_LOG(LogClass, Error, TEXT("MoveTo request failed due missing PathFollowingComponent"));
		return ResultData;
	}

	UMyPathFollowingComponent* PathFollowingComponent = m_CachePathFollowingComponent.Get();

	TObjectPtr<APawn> Pawn = GetPawn();
	if (Pawn == nullptr || Pawn->IsValidLowLevel() == false)
	{
		UE_LOG(LogClass, Error, TEXT("Owner pawn[APawn] is invalid"));
		return ResultData;
	}

	bool bCanRequestMove = ProcessCanRequestMove(MoveRequest);
	bool bAlreadyAtGoal = false;
	if (bCanRequestMove == true)
	{
		bAlreadyAtGoal = CheckAlreadyAtGoal(MoveRequest);
	}

	if (bAlreadyAtGoal == true)
	{
		ResultData.MoveId = PathFollowingComponent->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		ResultData.Code = EPathFollowingRequestResult::AlreadyAtGoal;
	}

	else if(bCanRequestMove == true)
	{
		// 비동기 길 찾기 요청
		TObjectPtr<UMyNavigationSystem> MyNavSystem = UMyFunctionHelpers::GetInstance<UMyNavigationSystem>(GetWorld());
		if (MyNavSystem != nullptr)
		{
			FPathFindingQuery PFQuery;
			const bool bValidQuery = BuildPathfindingQuery(MoveRequest, PFQuery);
			if (bValidQuery == true)
			{
				uint32&& AsyncQueryID = MyNavSystem->FindPathAsync(GetNavAgentPropertiesRef(), PFQuery, FNavPathQueryDelegate::CreateUObject(this, &URequestMoveComponent::OnPathFindingAsync));
				if (AsyncQueryID != INVALID_NAVQUERYID)
				{
					ResultData.QueryID = AsyncQueryID;
					ResultData.Code = EPathFollowingRequestResult::RequestSuccessful;
				}
			}
		}

		else
		{ 
			UE_LOG(LogClass, Error, TEXT("Fail to get [UMyNavigationSystem]"));
		}
	}

	if (ResultData.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		m_AsyncMoveParams.QueryID = ResultData.QueryID;
		m_AsyncMoveParams.MoveRequest = MoveRequest;
	}

	return ResultData;
}

void URequestMoveComponent::StopMovement(FAIRequestID InRequestID, EPathFollowingVelocityMode InVelocityMode)
{
	if (m_CachePathFollowingComponent.IsValid() == false)
	{
		return;
	}

	m_CachePathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::MovementStop | FPathFollowingResultFlags::ForcedScript, InRequestID, InVelocityMode);
}

void URequestMoveComponent::StopCurrentMovement(EPathFollowingVelocityMode InVelocityMode)
{
	AbortAsyncNavQuery();

	if (m_CachePathFollowingComponent.IsValid() == false)
	{
		return;
	}

	m_CachePathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::MovementStop | FPathFollowingResultFlags::ForcedScript, FAIRequestID::CurrentRequest, InVelocityMode);
}

void URequestMoveComponent::AbortAsyncNavQuery()
{
	if (m_AsyncMoveParams.IsValid() == true)
	{
		TObjectPtr<UMyNavigationSystem> MyNavSystem = UMyFunctionHelpers::GetInstance<UMyNavigationSystem>(GetWorld());
		if (MyNavSystem != nullptr)
		{
			MyNavSystem->AbortAsyncFindPathRequest(m_AsyncMoveParams.QueryID);
		}
		m_AsyncMoveParams.Clear();
	}
}

bool URequestMoveComponent::ProcessCanRequestMove(const FAIMoveRequest& InMoveRequest)
{
	bool bCanRequestMove = true;
	if (!InMoveRequest.IsMoveToActorRequest() == true)
	{
		if (InMoveRequest.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(InMoveRequest.GetGoalLocation()) == false)
		{
			UE_LOG(LogClass, Error, TEXT("URequestMoveComponent::MoveTo: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(InMoveRequest.GetGoalLocation()));
			bCanRequestMove = false;
		}

		if (bCanRequestMove == true && InMoveRequest.IsProjectingGoal() == true)
		{
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			const FNavAgentProperties& AgentProps = GetNavAgentPropertiesRef();
			FNavLocation ProjectedLocation;

			if (NavSys && !NavSys->ProjectPointToNavigation(InMoveRequest.GetGoalLocation(), ProjectedLocation, INVALID_NAVEXTENT, &AgentProps))
			{
				UE_LOG(LogClass, Error, TEXT("Fail to project point to navigation. location is [%s]"), *InMoveRequest.GetGoalLocation().ToString());

				bCanRequestMove = false;
			}

			InMoveRequest.UpdateGoalLocation(ProjectedLocation.Location);
		}
	}
	return bCanRequestMove;
}

bool URequestMoveComponent::CheckAlreadyAtGoal(const FAIMoveRequest& InMoveRequest)
{
	if (m_CachePathFollowingComponent.IsValid() == false || m_CachePathFollowingComponent->IsValidLowLevel() == false)
	{
		return false;
	}
	return m_CachePathFollowingComponent->HasReached(InMoveRequest);
}

// protected ====
void URequestMoveComponent::OnRegister()
{
	Super::OnRegister();

	// Find PathFollowingComponent
	m_CachePathFollowingComponent.Reset();

	TObjectPtr<AActor> OwnerActor = GetOwner();
	if (OwnerActor != nullptr)
	{
		m_CachePathFollowingComponent = Cast<UMyPathFollowingComponent>(OwnerActor->GetComponentByClass(UMyPathFollowingComponent::StaticClass()));
	}
}

void URequestMoveComponent::OnUnregister()
{
	AbortAsyncNavQuery();

	Super::OnUnregister();
}

// private ====
void URequestMoveComponent::ProcessBeforeRequestMove()
{

}

bool URequestMoveComponent::BuildPathfindingQuery(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& OutQuery) const
{
	bool bResult = false;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	const ANavigationData* NavData = (NavSys == nullptr) ? nullptr :
		InMoveRequest.IsUsingPathfinding() ? NavSys->GetNavDataForProps(GetNavAgentPropertiesRef(), GetNavAgentLocation()) :
		NavSys->GetAbstractNavData();

	if (NavData)
	{
		FVector GoalLocation = InMoveRequest.GetGoalLocation();
		if (InMoveRequest.IsMoveToActorRequest())
		{
			const INavAgentInterface* NavGoal = Cast<const INavAgentInterface>(InMoveRequest.GetGoalActor());
			if (NavGoal)
			{
				const FVector Offset = NavGoal->GetMoveGoalOffset(GetPawn());
				GoalLocation = FQuatRotationTranslationMatrix(InMoveRequest.GetGoalActor()->GetActorQuat(), NavGoal->GetNavAgentLocation()).TransformPosition(Offset);
			}
			else
			{
				GoalLocation = InMoveRequest.GetGoalActor()->GetActorLocation();
			}
		}

		FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, this, InMoveRequest.GetNavigationFilter());
		OutQuery = FPathFindingQuery(GetPawn(), *NavData, GetNavAgentLocation(), GoalLocation, NavFilter);
		OutQuery.SetAllowPartialPaths(InMoveRequest.IsUsingPartialPaths());

		if (m_CachePathFollowingComponent.IsValid() == true)
		{
			m_CachePathFollowingComponent->OnPathfindingQuery(OutQuery);
		}

		bResult = true;
	}

	else
	{
		
	}

	return bResult;
}

void URequestMoveComponent::FindPathForMoveRequest(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& InQuery, FNavPathSharedPtr& OutPath) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_Overall);

	TObjectPtr<UMyNavigationSystem> MyNavSystem = UMyFunctionHelpers::GetInstance<UMyNavigationSystem>(GetWorld());
	if (MyNavSystem != nullptr)
	{
		FPathFindingResult PathResult = MyNavSystem->FindPathSync(InQuery);
		if (PathResult.Result != ENavigationQueryResult::Error)
		{
			if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
			{
				if (InMoveRequest.IsMoveToActorRequest())
				{
					PathResult.Path->SetGoalActorObservation(*InMoveRequest.GetGoalActor(), 100.0f);
				}

				PathResult.Path->EnableRecalculationOnInvalidation(true);
				OutPath = PathResult.Path;
			}
		}
	}
}

void URequestMoveComponent::OnPathFindingAsync(uint32 InQueryID, ENavigationQueryResult::Type InResult, FNavPathSharedPtr InPath)
{
	if (m_AsyncMoveParams.QueryID == InQueryID)
	{
		m_AsyncMoveParams.Clear();

		if (m_CachePathFollowingComponent.IsValid() == true)
		{
			if (InPath.IsValid() == true)
			{
				const FAIRequestID RequestID = m_CachePathFollowingComponent->RequestMove(m_AsyncMoveParams.MoveRequest, InPath);
				if (RequestID.IsValid() == true)
				{
					// UE_LOG(LogClass, Log, TEXT("URequestMoveComponent::OnPathFindingAsync() : Success to request move"));

					TObjectPtr<UMyNavigationSystem> MyNavSystem = UMyFunctionHelpers::GetInstance<UMyNavigationSystem>(GetWorld());
					if (MyNavSystem != nullptr)
					{
						MyNavSystem->DrawPath(InPath);
					}
				}

				else
				{
					UE_LOG(LogClass, Log, TEXT("URequestMoveComponent::OnPathFindingAsync() : Fail to request move"));
				}
			}

			else
			{
				UE_LOG(LogClass, Warning, TEXT("URequestMoveComponent::OnPathFindingAsync() : Invalid Path"));
			}
		}

		else
		{
			UE_LOG(LogClass, Warning, TEXT("URequestMoveComponent::OnPathFindingAsync() : Invalid [UMyPathFollowingComponent]"));
		}
	}

	else
	{
		UE_LOG(LogClass, Log, TEXT("URequestMoveComponent::OnPathFindingAsync() : QueryID[%d] CurrentQueryID[%d], ignore"), InQueryID, m_AsyncMoveParams.QueryID);
	}
}

TObjectPtr<APawn> URequestMoveComponent::GetPawn() const
{
	TObjectPtr<APawn> PawnOwner = Cast<APawn>(GetOwner());
	if (PawnOwner == nullptr || PawnOwner->IsValidLowLevel() == false)
	{ 
		PawnOwner = nullptr;

		TObjectPtr<AController> Controller = Cast<AController>(GetOwner());
		if (Controller != nullptr && Controller->IsValidLowLevel() == true)
		{
			PawnOwner = Controller->GetPawn();
		}
	}

	return PawnOwner;
}

const FNavAgentProperties& URequestMoveComponent::GetNavAgentPropertiesRef() const
{
	TObjectPtr<APawn> Pawn = GetPawn();
	return Pawn ? Pawn->GetNavAgentPropertiesRef() : FNavAgentProperties::DefaultProperties;
}

FVector URequestMoveComponent::GetNavAgentLocation() const
{
	TObjectPtr<APawn> Pawn = GetPawn();
	return Pawn ? Pawn->GetNavAgentLocation() : FAISystem::InvalidLocation;
}