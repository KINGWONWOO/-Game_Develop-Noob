// OXQuizGameState.cpp

#include "OXQuizGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "QuizGame/OXQuizPlayerController.h"
#include "Kismet/GameplayStatics.h"

AOXQuizGameState::AOXQuizGameState()
{
    CurrentGamePhase = EQuizGamePhase::GP_WaitingToStart;
    Winner = nullptr;
	WinningCharacterType = ECharacterType::ECT_None;
    CurrentSpeedLevel = 0;
    PlayingCountdown = -1;
}

void AOXQuizGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOXQuizGameState, CurrentGamePhase);
    DOREPLIFETIME(AOXQuizGameState, Winner);
    DOREPLIFETIME(AOXQuizGameState, WinningCharacterType);
    DOREPLIFETIME(AOXQuizGameState, CurrentSpeedLevel);
    DOREPLIFETIME(AOXQuizGameState, PlayingCountdown);
}

void AOXQuizGameState::OnRep_GamePhase()
{
	// 게임 단계가 변경되었음을 UI(블루프린트)에 알립니다.
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);

	// 이 함수는 모든 클라이언트에서 실행됩니다.
	if (CurrentGamePhase == EQuizGamePhase::GP_GameOver)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_GamePhase: Detected GP_GameOver on Client."));


		// 로컬 플레이어 컨트롤러를 가져옵니다. (멀티플레이어에서는 0번 인덱스가 로컬 플레이어)
		AOXQuizPlayerController* PC = Cast<AOXQuizPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
		if (PC)
		{
			bool bAmIWinner = false;
			if (Winner && PC->PlayerState == Winner)
			{
				bAmIWinner = true;
			}

			UE_LOG(LogTemp, Warning, TEXT("OnRep_GamePhase: Calling Event_SetupResultsScreen and Event_ShowResultsScreen. WinnerType: %s"), *UEnum::GetValueAsString(WinningCharacterType));

			// 1. 카메라/입력 설정
			PC->Event_SetupResultsScreen();
			// 2. UI 및 애니메이션 설정
			PC->Event_ShowResultsScreen(WinningCharacterType, bAmIWinner);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("OnRep_GamePhase: FAILED to cast to AFruitPlayerController!"));
		}
	}
}

// --- Speed ---
void AOXQuizGameState::SetCurrentSpeedLevel(int32 NewLevel)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        if (CurrentSpeedLevel != NewLevel)
        {
            CurrentSpeedLevel = NewLevel;
            OnRep_CurrentSpeedLevel();
        }
    }
}
void AOXQuizGameState::OnRep_CurrentSpeedLevel()
{
    OnSpeedLevelChanged.Broadcast(CurrentSpeedLevel);
}

// --- Playing Countdown ---
void AOXQuizGameState::SetPlayingCountdown(int32 TimeLeft)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        PlayingCountdown = TimeLeft;
        OnRep_PlayingCountdown();
    }
}
void AOXQuizGameState::OnRep_PlayingCountdown()
{
    OnPlayingCountdownChanged.Broadcast(PlayingCountdown);
}