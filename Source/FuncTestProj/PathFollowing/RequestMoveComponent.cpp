// Fill out your copyright notice in the Description page of Project Settings.


#include "RequestMoveComponent.h"

#include "NavigationSystem.h"

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
	const bool bStopCurrentMovement = true;
	ProcessBeforeRequestMove(bStopCurrentMovement);

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

	bool bCanRequestMove = true;
	bool bAlreadyAtGoal = false;

	if (!MoveRequest.IsMoveToActorRequest() == true)
	{
		if (MoveRequest.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(MoveRequest.GetGoalLocation()) == false)
		{
			UE_LOG(LogClass, Error, TEXT("URequestMoveComponent::MoveTo: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
			bCanRequestMove = false;
		}

		if (bCanRequestMove == true && MoveRequest.IsProjectingGoal() == true)
		{
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			const FNavAgentProperties& AgentProps = GetNavAgentPropertiesRef();
			FNavLocation ProjectedLocation;

			if (NavSys && !NavSys->ProjectPointToNavigation(MoveRequest.GetGoalLocation(), ProjectedLocation, INVALID_NAVEXTENT, &AgentProps))
			{
				UE_LOG(LogClass, Error, TEXT("Fail to project point to navigation. location is [%s]"), *MoveRequest.GetGoalLocation().ToString());

				bCanRequestMove = false;
			}

			MoveRequest.UpdateGoalLocation(ProjectedLocation.Location);
		}

		bAlreadyAtGoal = bCanRequestMove && PathFollowingComponent->HasReached(MoveRequest);
	}

	else
	{
		bAlreadyAtGoal = bCanRequestMove && PathFollowingComponent->HasReached(MoveRequest);
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
	if (m_CachePathFollowingComponent.IsValid() == false)
	{
		return;
	}

	m_CachePathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::MovementStop | FPathFollowingResultFlags::ForcedScript, FAIRequestID::CurrentRequest, InVelocityMode);
}

void URequestMoveComponent::ProcessBeforeRequestMove(bool InStopCurrentMovement)
{
	if (InStopCurrentMovement == true)
	{
		StopCurrentMovement();
	}
}

bool URequestMoveComponent::BuildPathfindingQuery(const FAIMoveRequest& InMoveRequest, FPathFindingQuery& InQuery) const
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
		InQuery = FPathFindingQuery(GetPawn(), *NavData, GetNavAgentLocation(), GoalLocation, NavFilter);
		InQuery.SetAllowPartialPaths(InMoveRequest.IsUsingPartialPaths());

		if (m_CachePathFollowingComponent.IsValid() == true)
		{
			m_CachePathFollowingComponent->OnPathfindingQuery(InQuery);
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

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys != nullptr)
	{
		FPathFindingResult PathResult = NavSys->FindPathSync(InQuery);
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