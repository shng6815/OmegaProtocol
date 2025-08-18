// Copyright It's My Turn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "OmochaGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoteCountChanged, int32, CurrentVotes, int32, RequiredVotes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkipCinematicApproved);
/**
 * 
 */
UCLASS()
class OMOCHA_API AOmochaGameStateBase : public AGameState
{
	GENERATED_BODY()

public:
	AOmochaGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void StartCinematicSkipVoting();

	UFUNCTION(BlueprintCallable)
	void AddCinematicSkipVote();

	UFUNCTION(BlueprintCallable)
	void RemoveCinematicSkipVote();

	UFUNCTION(BlueprintCallable)
	void EndCinematicSkipVoting();

	UFUNCTION(BlueprintPure)
	void GetVoteStatus(int32& CurrentVotes, int32& RequiredVotes, bool& bIsActive) const;

	UFUNCTION(BlueprintCallable, Category = "Voting")
	int32 GetCurrentPlayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "Voting")
	void UpdateRequiredVotes();

	UPROPERTY(BlueprintAssignable)
	FOnVoteCountChanged OnVoteCountChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSkipCinematicApproved OnSkipCinematicApproved;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentSkipVotes)
	int32 CurrentSkipVotes;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly)
	int32 RequiredSkipVotes = 3;

	UPROPERTY(ReplicatedUsing = OnRep_VotingActive)
	bool bSkipVotingActive = false;

	UFUNCTION()
	void OnRep_CurrentSkipVotes();

	UFUNCTION()
	void OnRep_VotingActive();

private:
	void CheckVoteResult();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSkipApproved();
};
