// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class QIITASAMPLE_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Qiita Sample")
		FString Login();

	UFUNCTION(BlueprintCallable, Category = "Qiita Sample")
		bool HostSession();

	UFUNCTION(BlueprintCallable, Category = "Qiita Sample")
		void FindSession();

	UFUNCTION(BlueprintCallable, Category = "Qiita Sample")
		void KillSession();

	UFUNCTION(BlueprintCallable, Category = "Qiita Sample")
		void SendInvitation();

private:
	void OnLoginCompleteDelegate(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void JoinSession(FOnlineSessionSearchResult SearchResult);
	void OnJoinSessionCompleteDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnCreateSessionCompleteDelegate(FName InSessionName, bool bWasSuccessful);
	void OnFindSessionsCompleteDelegate(bool bWasSuccessful);
	void OnSessionUserInviteAcceptedDelegate(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void OnReadFriendsCompleteDelegate(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
};
