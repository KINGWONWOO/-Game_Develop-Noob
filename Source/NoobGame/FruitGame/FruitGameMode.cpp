// FruitGameMode.cpp

#include "FruitGame/FruitGameMode.h"
#include "FruitGame/FruitGameState.h"
#include "FruitGame/FruitPlayerState.h"
#include "FruitGame/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AFruitGameMode::AFruitGameMode()
{
	// 사용할 클래스들을 기본값으로 설정
	GameStateClass = AFruitGameState::StaticClass();
	PlayerStateClass = AFruitPlayerState::StaticClass();
	PlayerControllerClass = AFruitPlayerController::StaticClass();

	// 틱 활성화 (타이머 UI 갱신용)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f; // 너무 자주 틱할 필요 없음

	MyGameState = nullptr;
}

void AFruitGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// GameState 캐시
	if (!MyGameState)
	{
		MyGameState = GetGameState<AFruitGameState>();
	}

	// 2명이 모두 접속했는지 확인
	if (MyGameState && GetNumPlayers() == 2)
	{
		// 2명이 모두 접속하면 "준비" 단계로 전환
		MyGameState->CurrentGamePhase = EGamePhase::GP_Setup;
	}
}

void AFruitGameMode::PlayerSubmittedFruits(AController* PlayerController, const TArray<EFruitType>& SecretFruits)
{
	if (!MyGameState || MyGameState->CurrentGamePhase != EGamePhase::GP_Setup)
	{
		return; // 준비 단계가 아니면 무시
	}

	AFruitPlayerState* PS = PlayerController->GetPlayerState<AFruitPlayerState>();
	if (PS && !PS->bHasSubmittedFruits)
	{
		PS->SetSecretAnswers_Server(SecretFruits);
		CheckBothPlayersReady();
	}
}

void AFruitGameMode::CheckBothPlayersReady()
{
	if (!MyGameState) return;

	int32 ReadyPlayers = 0;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerState* FruitPS = Cast<AFruitPlayerState>(PS);
		if (FruitPS && FruitPS->bHasSubmittedFruits)
		{
			ReadyPlayers++;
		}
	}

	// 2명 모두 준비 완료 시, 동전 던지기 시작
	if (ReadyPlayers == 2)
	{
		StartCoinToss();
	}
}

void AFruitGameMode::StartCoinToss()
{
	if (!MyGameState) return;

	MyGameState->CurrentGamePhase = EGamePhase::GP_PlayerTurn;

	// 0 또는 1 랜덤 선택
	int32 FirstPlayerIndex = FMath::RandRange(0, 1);
	MyGameState->CurrentActivePlayer = MyGameState->PlayerArray[FirstPlayerIndex];

	StartTurn();
}

void AFruitGameMode::StartTurn()
{
	if (!MyGameState || !MyGameState->CurrentActivePlayer) return;

	// 턴 타이머 설정
	MyGameState->TurnTimer = TurnDuration;

	// 타이머 핸들 설정 (시간 만료 시 OnTurnTimerExpired 호출)
	GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &AFruitGameMode::OnTurnTimerExpired, TurnDuration);

	// 현재 턴인 플레이어의 컨트롤러 찾기
	AFruitPlayerController* ActivePC = Cast<AFruitPlayerController>(MyGameState->CurrentActivePlayer->GetPlayerController());
	if (ActivePC)
	{
		// 해당 클라이언트에게 "당신 턴"임을 알림
		ActivePC->Client_StartTurn();
	}
}

void AFruitGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 턴 진행 중일 때 GameState의 타이머 값을 계속 감소 (UI 표시용)
	if (MyGameState && MyGameState->CurrentGamePhase == EGamePhase::GP_PlayerTurn)
	{
		MyGameState->TurnTimer -= DeltaSeconds;
		if (MyGameState->TurnTimer < 0.0f)
		{
			MyGameState->TurnTimer = 0.0f;
		}
	}
}

void AFruitGameMode::OnTurnTimerExpired()
{
	// 시간 초과로 턴 종료
	EndTurn(true);
}

bool AFruitGameMode::IsPlayerTurn(AController* PlayerController) const
{
	if (!MyGameState || !PlayerController)
	{
		return false;
	}
	return MyGameState->CurrentActivePlayer == PlayerController->PlayerState;
}

void AFruitGameMode::ProcessPlayerGuess(AController* PlayerController, const TArray<EFruitType>& GuessedFruits)
{
	if (!IsPlayerTurn(PlayerController))
	{
		return; // 현재 턴인 플레이어가 아니면 무시
	}

	// 턴 타이머 정지
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	MyGameState->TurnTimer = 0.0f; // UI 타이머도 0으로

	// 1. 상대방(정답 소유자) 찾기
	AFruitPlayerState* OpponentPS = nullptr;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		if (PS != PlayerController->PlayerState)
		{
			OpponentPS = Cast<AFruitPlayerState>(PS);
			break;
		}
	}

	if (!OpponentPS) return; // 상대가 없으면 처리 불가

	// 2. 상대방의 정답(서버에만 있음) 가져오기
	const TArray<EFruitType>& OpponentSecret = OpponentPS->GetSecretAnswers_Server();

	// 3. 정답 비교
	int32 MatchCount = 0;
	for (int32 i = 0; i < 5; ++i)
	{
		if (GuessedFruits.IsValidIndex(i) && OpponentSecret.IsValidIndex(i) &&
			GuessedFruits[i] == OpponentSecret[i] && GuessedFruits[i] != EFruitType::FT_None)
		{
			MatchCount++;
		}
	}

	// 4. 결과 전송
	AFruitPlayerController* GuesserPC = Cast<AFruitPlayerController>(PlayerController);
	AFruitPlayerController* OpponentPC = Cast<AFruitPlayerController>(OpponentPS->GetPlayerController());

	if (GuesserPC)
	{
		// 추측한 사람에게 결과 전송
		GuesserPC->Client_ReceiveGuessResult(GuessedFruits, MatchCount);
	}
	if (OpponentPC)
	{
		// 상대방에게 추측 내용 전송
		OpponentPC->Client_OpponentGuessed(GuessedFruits, MatchCount);
	}

	// 5. 승리 판정 또는 턴 넘기기
	if (MatchCount == 5)
	{
		EndGame(PlayerController->PlayerState); // 추측한 사람 승리
	}
	else
	{
		EndTurn(false); // 시간 초과가 아님
	}
}

void AFruitGameMode::EndTurn(bool bTimeOut)
{
	if (!MyGameState) return;

	// (bTimeOut이 true일 경우 추가 로직... 예: 페널티)

	// 다음 턴 플레이어 찾기
	APlayerState* NextPlayer = nullptr;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		if (PS != MyGameState->CurrentActivePlayer)
		{
			NextPlayer = PS;
			break;
		}
	}

	if (NextPlayer)
	{
		MyGameState->CurrentActivePlayer = NextPlayer;
		StartTurn(); // 다음 턴 시작
	}
	else
	{
		// 플레이어가 1명뿐이거나 오류 발생 시
		EndGame(nullptr); // 무승부 또는 오류 처리
	}
}

void AFruitGameMode::EndGame(APlayerState* Winner)
{
	if (!MyGameState) return;

	MyGameState->CurrentGamePhase = EGamePhase::GP_GameOver;
	MyGameState->Winner = Winner;

	// 턴 타이머 완전 정지
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	MyGameState->TurnTimer = 0.0f;

	// 모든 플레이어에게 게임 종료 및 승패 여부 전송
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
		if (PC)
		{
			PC->Client_GameOver(PS == Winner);
		}
	}

	// (필요시 10초 뒤 로비로 ServerTravel 등)
}