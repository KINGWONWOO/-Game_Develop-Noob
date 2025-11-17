// OXQuizPlayerState.cpp
#include "OXQuizPlayerState.h"
#include "Net/UnrealNetwork.h"

AOXQuizPlayerState::AOXQuizPlayerState()
{
    bIsReady_Instructions = false;
    PunchHitCount = 0;
    bIsKnockedDown = false;
    bIsNextPunchLeft = true; // 첫 펀치는 왼쪽
}

void AOXQuizPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOXQuizPlayerState, bIsReady_Instructions);
    DOREPLIFETIME(AOXQuizPlayerState, PunchHitCount);
    DOREPLIFETIME(AOXQuizPlayerState, bIsKnockedDown);
    DOREPLIFETIME(AOXQuizPlayerState, bIsNextPunchLeft);
}

// --- Ready ---
void AOXQuizPlayerState::SetInstructionReady_Server()
{
    if (HasAuthority())
    {
        bIsReady_Instructions = true;
        // OnRep은 클라이언트에서 자동 호출
    }
}