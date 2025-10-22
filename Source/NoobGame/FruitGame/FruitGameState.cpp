// FruitGameState.cpp

#include "FruitGame/FruitGameState.h"
#include "Net/UnrealNetwork.h"

AFruitGameState::AFruitGameState()
{
	// 기본값 초기화
	CurrentGamePhase = EGamePhase::GP_WaitingToStart;
	CurrentActivePlayer = nullptr;
	TurnTimer = 0.0f;
	Winner = nullptr;
}

void AFruitGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 변수들을 복제 등록
	DOREPLIFETIME(AFruitGameState, CurrentGamePhase);
	DOREPLIFETIME(AFruitGameState, CurrentActivePlayer);
	DOREPLIFETIME(AFruitGameState, TurnTimer);
	DOREPLIFETIME(AFruitGameState, Winner);
}

void AFruitGameState::OnRep_GamePhase()
{
	// 게임 단계가 변경되었음을 UI(블루프린트)에 알립니다.
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}