// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EngineGlobals.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#define DISPLAY_LOG(fmt, ...) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT(fmt), __VA_ARGS__));

const FName SESSION_NAME = "SessionName";

TSharedPtr<class FOnlineSessionSearch> SearchSettings;

FString AMyPlayerController::Login()
{

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid())
		{
			ULocalPlayer* LP = Cast<ULocalPlayer>(Player);
			if (LP != NULL)
			{
				int ControllerId = LP->GetControllerId();
				if (Identity->GetLoginStatus(ControllerId) != ELoginStatus::LoggedIn)
				{
					UE_LOG_ONLINE(Warning, TEXT("CommandLine: %s"), FCommandLine::Get());

					Identity->AddOnLoginCompleteDelegate_Handle(ControllerId, FOnLoginCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnLoginCompleteDelegate));
					Identity->AutoLogin(ControllerId);

				}

				ELoginStatus::Type status = Identity->GetLoginStatus(ControllerId);
				DISPLAY_LOG("Login Status: %s", ELoginStatus::ToString(status));

				
			}
		}
	}

	return "";
}

bool AMyPlayerController::HostSession()
{
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			TSharedPtr<class FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
			SessionSettings->NumPublicConnections = 4;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bUsesPresence = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bUseLobbiesIfAvailable = true;
			SessionSettings->bUseLobbiesVoiceChatIfAvailable = true;
			SessionSettings->Set(SEARCH_KEYWORDS, FString("Custom"), EOnlineDataAdvertisementType::ViaOnlineService);

			Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnCreateSessionCompleteDelegate));

			TSharedPtr<const FUniqueNetId> UniqueNetIdptr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			bool bResult = Sessions->CreateSession(*UniqueNetIdptr, SESSION_NAME, *SessionSettings);

			if (bResult) {
				// DISPLAY_LOG("CreateSession: Success");
				return true;
			}
			else {
				DISPLAY_LOG("CreateSession: Fail");
			}

		}
	}

	return false;
}

void AMyPlayerController::OnCreateSessionCompleteDelegate(FName InSessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThirdPersonBP/Maps/ThirdPersonExampleMap")), true, "listen");
	}
}

void AMyPlayerController::FindSession()
{
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			SearchSettings = MakeShareable(new FOnlineSessionSearch());
			SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Custom"), EOnlineComparisonOp::Equals);

			Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnFindSessionsCompleteDelegate));

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();
			TSharedPtr<const FUniqueNetId> UniqueNetIdptr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			bool bIsSuccess = Sessions->FindSessions(*UniqueNetIdptr, SearchSettingsRef);
		}
	}
}

void AMyPlayerController::OnFindSessionsCompleteDelegate(bool bWasSuccessful) {
	if (bWasSuccessful) {
		DISPLAY_LOG("Find Session: Success");
		if (SearchSettings->SearchResults.Num() == 0) {
			DISPLAY_LOG("No session found.");
		}
		else {
			JoinSession(SearchSettings->SearchResults[0]);
		}
	}
	else {
		DISPLAY_LOG("Find Session: Fail");
	}
}

void AMyPlayerController::OnSessionInviteReceivedDelegate(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	const TCHAR* ToUserId = *UserId.ToString();
	const TCHAR* FromUserId = *FromId.ToString();
	DISPLAY_LOG("Received Invite from %s to %s", ToUserId, FromUserId);
}

void AMyPlayerController::OnReadFriendsCompleteDelegate(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	DISPLAY_LOG("FRIEND_LIST RETRIEVED");
}

void AMyPlayerController::JoinSession(FOnlineSessionSearchResult SearchResult) {
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (SearchResult.IsValid()) {
				Sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnJoinSessionCompleteDelegate));

				TSharedPtr<const FUniqueNetId> UniqueNetIdptr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
				Sessions->JoinSession(*UniqueNetIdptr, SESSION_NAME, SearchResult);
				// DISPLAY_LOG("Join Session");
			}
			else {
				DISPLAY_LOG("Invalid session.");
			}
		}
	}
}

void AMyPlayerController::OnJoinSessionCompleteDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (Result == EOnJoinSessionCompleteResult::Success)
			{
				// Client travel to the server
				FString ConnectString;
				if (Sessions->GetResolvedConnectString(SESSION_NAME, ConnectString))
				{
					UE_LOG_ONLINE_SESSION(Log, TEXT("Join session: traveling to %s"), *ConnectString);
					AMyPlayerController::ClientTravel(ConnectString, TRAVEL_Absolute);
				}
			}
		}
	}
}

void AMyPlayerController::KillSession()
{
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->DestroySession(SESSION_NAME);
			UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThirdPersonBP/Maps/ThirdPersonExampleMap")), true, "");
		}
	}
}

void AMyPlayerController::OnLoginCompleteDelegate(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineIdentityPtr Identity = Online::GetIdentityInterface();
	if (Identity.IsValid())
	{
		ULocalPlayer* LP = Cast<ULocalPlayer>(Player);
		if (LP != NULL)
		{
			int ControllerId = LP->GetControllerId();
			FUniqueNetIdRepl uniqueId = PlayerState->GetUniqueId();
			uniqueId.SetUniqueNetId(FUniqueNetIdWrapper(UserId).GetUniqueNetId());
			PlayerState->SetUniqueId(uniqueId);
		}
	}

	IOnlineSessionPtr Session = Online::GetSessionInterface();
	if (Session.IsValid())
	{
		Session->AddOnSessionInviteReceivedDelegate_Handle(FOnSessionInviteReceivedDelegate::CreateUObject(this, &AMyPlayerController::OnSessionInviteReceivedDelegate));
	}

	IOnlineFriendsPtr Friends = Online::GetFriendsInterface();
	if (Friends.IsValid())
	{
		Friends->ReadFriendsList(0, SESSION_NAME.ToString(), FOnReadFriendsListComplete::CreateUObject(this, &AMyPlayerController::OnReadFriendsCompleteDelegate));
	}
}

void AMyPlayerController::SendInvitation()
{
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineFriendsPtr Friends = OnlineSub->GetFriendsInterface();
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && Friends.IsValid())
		{
			TArray<TSharedRef<FOnlineFriend>> FriendList;
			Friends->GetFriendsList(0,SESSION_NAME.ToString(), FriendList);
			for (TSharedRef<FOnlineFriend> Friend : FriendList)
			{
				FUniqueNetIdRef FriendUniqueNetIdRef = Friend->GetUserId();
				Sessions->SendSessionInviteToFriend(0, SESSION_NAME, FriendUniqueNetIdRef.Get());
			}
		}
	}
}