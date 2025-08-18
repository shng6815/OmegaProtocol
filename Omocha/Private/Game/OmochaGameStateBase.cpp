// Copyright It's My Turn


#include "Game/OmochaGameStateBase.h"

#include "Net/UnrealNetwork.h"
#include "Player/OmochaPlayerState.h"

AOmochaGameStateBase::AOmochaGameStateBase()
{
	RequiredSkipVotes = 1;
	CurrentSkipVotes = 0;
	bSkipVotingActive = false;
}

void AOmochaGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOmochaGameStateBase, CurrentSkipVotes);
	DOREPLIFETIME(AOmochaGameStateBase, RequiredSkipVotes);
	DOREPLIFETIME(AOmochaGameStateBase, bSkipVotingActive);
}

void AOmochaGameStateBase::StartCinematicSkipVoting()
{
	if (HasAuthority())
	{
		CurrentSkipVotes = 0;

		UpdateRequiredVotes();
		
		bSkipVotingActive = true;
	}
}

void AOmochaGameStateBase::AddCinematicSkipVote()
{
	if (HasAuthority() && bSkipVotingActive)
	{
		CurrentSkipVotes++;
		CheckVoteResult();
	}
}

void AOmochaGameStateBase::RemoveCinematicSkipVote()
{
	if (HasAuthority() && bSkipVotingActive && CurrentSkipVotes > 0)
	{
		CurrentSkipVotes--;
	}
}

void AOmochaGameStateBase::EndCinematicSkipVoting()
{
	if (HasAuthority())
	{
		CurrentSkipVotes = 0;
		bSkipVotingActive = false;
	}
}

void AOmochaGameStateBase::GetVoteStatus(int32& CurrentVotes, int32& RequiredVotes, bool& bIsActive) const
{
	CurrentVotes = CurrentSkipVotes;
	RequiredVotes = RequiredSkipVotes;
	bIsActive = bSkipVotingActive;
}

int32 AOmochaGameStateBase::GetCurrentPlayerCount() const
{
	int32 PlayerCount = 0;

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (PlayerState && !PlayerState->IsInactive())
		{
			PlayerCount++;
		}
	}

	return PlayerCount;
}

void AOmochaGameStateBase::UpdateRequiredVotes()
{
	if (HasAuthority())
	{
		int32 CurrentPlayerCount = GetCurrentPlayerCount();
		RequiredSkipVotes = FMath::Max(1, CurrentPlayerCount);
	}
}

void AOmochaGameStateBase::OnRep_CurrentSkipVotes()
{
	OnVoteCountChanged.Broadcast(CurrentSkipVotes, RequiredSkipVotes);
}

void AOmochaGameStateBase::OnRep_VotingActive()
{
	if (bSkipVotingActive)
	{
		OnVoteCountChanged.Broadcast(CurrentSkipVotes, RequiredSkipVotes);
	}
}

void AOmochaGameStateBase::CheckVoteResult()
{
	if (HasAuthority() && CurrentSkipVotes >= RequiredSkipVotes)
	{
		Multicast_OnSkipApproved();
		EndCinematicSkipVoting();
	}
}

void AOmochaGameStateBase::Multicast_OnSkipApproved_Implementation()
{
	OnSkipCinematicApproved.Broadcast();
}
