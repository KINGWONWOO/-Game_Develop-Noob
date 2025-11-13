// OXQuizGameState.cpp

#include "OXQuizGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

AOXQuizGameState::AOXQuizGameState()
{
    CurrentPhase = EQuizGamePhase::WaitingToStart;
    Winner = nullptr;
    Loser = nullptr;
}

void AOXQuizGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOXQuizGameState, CurrentPhase);
    DOREPLIFETIME(AOXQuizGameState, Winner);
    DOREPLIFETIME(AOXQuizGameState, Loser);
}

void AOXQuizGameState::SetCurrentPhase(EQuizGamePhase NewPhase)
{
    // 서버에서만 호출되어야 합니다.
    if (GetLocalRole() == ROLE_Authority)
    {
        CurrentPhase = NewPhase;
        OnRep_CurrentPhase(); // 서버 자신도 호출
    }
}

void AOXQuizGameState::OnRep_CurrentPhase()
{
    // 클라이언트 UI 갱신 등
    // 예: FString PhaseString = UEnum::GetValueAsString(CurrentPhase);
    // UE_LOG(LogTemp, Log, TEXT("Client: GamePhase changed to %s"), *PhaseString);
}

void AOXQuizGameState::Server_SetWinnerAndLoser_Implementation(APlayerState* NewWinner, APlayerState* NewLoser)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        Winner = NewWinner;
        Loser = NewLoser;
    }
}