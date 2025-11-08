// FruitGameState.cpp

#include "FruitGame/FruitGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "FruitGame/FruitPlayerController.h" // (신규!) PlayerController 참조
#include "Kismet/GameplayStatics.h" // (신규!) UGameplayStatics 참조

AFruitGameState::AFruitGameState()
{
	// 기본값 초기화
	CurrentGamePhase = EGamePhase::GP_WaitingToStart;
	CurrentActivePlayer = nullptr;
	ServerTimeAtTurnStart = 0.0f;
	Winner = nullptr;
	WinningCharacterType = ECharacterType::ECT_None;
}

/** 이 함수는 GameState의 변수를 복제해야 합니다. */
void AFruitGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// .h 파일에 선언된 변수들을 복제합니다.
	DOREPLIFETIME(AFruitGameState, CurrentGamePhase);
	DOREPLIFETIME(AFruitGameState, CurrentActivePlayer);
	DOREPLIFETIME(AFruitGameState, ServerTimeAtTurnStart);
	DOREPLIFETIME(AFruitGameState, Winner);
	DOREPLIFETIME(AFruitGameState, WinningCharacterType);
}

/** (수정!) OnRep_GamePhase가 게임 종료 로직을 호출합니다. */
void AFruitGameState::OnRep_GamePhase()
{
	// 게임 단계가 변경되었음을 UI(블루프린트)에 알립니다.
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);

	// --- (신규!) 게임 종료 연출 시작 ---
	// 이 함수는 모든 클라이언트에서 실행됩니다.
	if (CurrentGamePhase == EGamePhase::GP_GameOver)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_GamePhase: Detected GP_GameOver on Client."));

		// 로컬 플레이어 컨트롤러를 가져옵니다. (멀티플레이어에서는 0번 인덱스가 로컬 플레이어)
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
		if (PC)
		{
			bool bAmIWinner = false;
			if (Winner && PC->PlayerState == Winner)
			{
				bAmIWinner = true;
			}

			// (중요!) 이 시점에는 WinningCharacterType이 100% 복제되어 있습니다.
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

void AFruitGameState::OnRep_CurrentActivePlayer()
{
	// (사용자 제공 로그 코드)
	APlayerState* LocalPlayerState = nullptr;
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		LocalPlayerState = GetWorld()->GetFirstPlayerController()->PlayerState;
		if (LocalPlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("[OnRep_CurrentActivePlayer] -- Client: %s --"), *LocalPlayerState->GetPlayerName());
		}
	}

	if (CurrentGamePhase == EGamePhase::GP_PlayerTurn && CurrentActivePlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("   CurrentActivePlayer: %s (Addr: %p)"), *CurrentActivePlayer->GetPlayerName(), (void*)CurrentActivePlayer);

		UE_LOG(LogTemp, Warning, TEXT("   PlayerArray Order:"));
		for (int32 i = 0; i < PlayerArray.Num(); ++i)
		{
			if (PlayerArray[i])
			{
				UE_LOG(LogTemp, Warning, TEXT("     [%d]: %s (Addr: %p)"), i, *PlayerArray[i]->GetPlayerName(), (void*)PlayerArray[i]);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("     [%d]: nullptr"), i);
			}
		}

		int32 StartingPlayerIndex = INDEX_NONE;
		for (int32 i = 0; i < PlayerArray.Num(); ++i)
		{
			if (PlayerArray[i] == CurrentActivePlayer)
			{
				StartingPlayerIndex = i;
				break;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("   Calculated StartingPlayerIndex: %d"), StartingPlayerIndex);

		if (StartingPlayerIndex != INDEX_NONE)
		{
			OnFirstTurnPlayerDetermined.Broadcast(StartingPlayerIndex);
			UE_LOG(LogTemp, Warning, TEXT("   Broadcasted Index: %d"), StartingPlayerIndex);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("   ERROR: CurrentActivePlayer not found in PlayerArray!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnRep_CurrentActivePlayer] CurrentGamePhase is not GP_PlayerTurn or CurrentActivePlayer is invalid."));
	}
}