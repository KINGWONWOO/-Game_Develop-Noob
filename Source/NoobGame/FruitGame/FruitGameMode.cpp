// FruitGameMode.cpp

#include "FruitGame/FruitGameMode.h"
#include "FruitGame/FruitGameState.h"
#include "FruitGame/FruitPlayerState.h"
#include "FruitGame/FruitPlayerController.h"
#include "FruitGame/InteractableFruitObject.h"
#include "FruitGame/SubmitGuessButton.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h" // TurnTimerHandle을 위해 유지

AFruitGameMode::AFruitGameMode()
{
	GameStateClass = AFruitGameState::StaticClass();
	PlayerStateClass = AFruitPlayerState::StaticClass();
	PlayerControllerClass = AFruitPlayerController::StaticClass();
	PrimaryActorTick.bCanEverTick = false; // 틱 사용 안 함
	MyGameState = nullptr;
	NumPlayersReady_Setup = 0;
	SpinnerResultIndex = -1;
}

void AFruitGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!MyGameState)
	{
		MyGameState = GetGameState<AFruitGameState>();
	}
	if (MyGameState && GetNumPlayers() == 2)
	{
		MyGameState->CurrentGamePhase = EGamePhase::GP_Instructions;
	}
}

// (GP_PlayerTurn 전용 상호작용)
void AFruitGameMode::PlayerInteracted(AController* PlayerController, AActor* HitActor, EGamePhase CurrentPhase)
{
	if (CurrentPhase == EGamePhase::GP_PlayerTurn)
	{
		if (!IsPlayerTurn(PlayerController)) return;

		if (AInteractableFruitObject* GuessObject = Cast<AInteractableFruitObject>(HitActor))
		{
			GuessObject->CycleFruit();
		}
		else if (ASubmitGuessButton* GuessSubmitButton = Cast<ASubmitGuessButton>(HitActor))
		{
			ProcessGuessFromWorldObjects(PlayerController);
		}
	}
}

// --- 1. Instructions 단계 ---
void AFruitGameMode::PlayerIsReady(AController* PlayerController)
{
	if (!MyGameState || MyGameState->CurrentGamePhase != EGamePhase::GP_Instructions)
	{
		return;
	}
	AFruitPlayerState* PS = PlayerController->GetPlayerState<AFruitPlayerState>();
	if (PS)
	{
		PS->SetInstructionReady_Server();
		CheckBothPlayersReady_Instructions();
	}
}

void AFruitGameMode::CheckBothPlayersReady_Instructions()
{
	if (!MyGameState) return;
	int32 ReadyPlayers = 0;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerState* FruitPS = Cast<AFruitPlayerState>(PS);
		if (FruitPS && FruitPS->bIsReady_Instructions)
		{
			ReadyPlayers++;
		}
	}
	if (ReadyPlayers == 2)
	{
		MyGameState->CurrentGamePhase = EGamePhase::GP_Setup;
	}
}

// --- 2. Setup 단계 ---
void AFruitGameMode::PlayerSubmittedFruits(AController* PlayerController, const TArray<EFruitType>& SecretFruits)
{
	if (!MyGameState || MyGameState->CurrentGamePhase != EGamePhase::GP_Setup)
	{
		return;
	}
	AFruitPlayerState* PS = PlayerController->GetPlayerState<AFruitPlayerState>();
	if (PS && !PS->bHasSubmittedFruits)
	{
		PS->SetSecretAnswers_Server(SecretFruits);
		NumPlayersReady_Setup++;
		CheckBothPlayersReady_Setup();
	}
}

/** (수정) Setup 완료 시 돌림판 단계 시작 */
void AFruitGameMode::CheckBothPlayersReady_Setup()
{
	if (NumPlayersReady_Setup == 2)
	{
		StartSpinnerPhase();
	}
}

// --- 3. SpinnerTurn 단계 ---

/** (신규) 돌림판 단계 시작 */
void AFruitGameMode::StartSpinnerPhase()
{
	if (!MyGameState) return;

	// 1. GameState를 돌림판 단계로 변경 (UI가 감지하도록)
	MyGameState->CurrentGamePhase = EGamePhase::GP_SpinnerTurn;

	// 2. 서버에서 미리 결과 결정 (0 또는 1)
	SpinnerResultIndex = FMath::RandRange(0, 1);

	// 3. 모든 클라이언트에게 애니메이션 재생 지시 (결과 포함)
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
		if (PC)
		{
			// 클라이언트 RPC 호출 (이 RPC는 BlueprintImplementableEvent를 호출함)
			PC->Client_PlaySpinnerAnimation(SpinnerResultIndex);
		}
	}
	// (삭제) 타이머로 대기하는 로직 삭제.
}

/** (신규!) 블루프린트(UI)의 요청을 받아 실제 턴을 시작하는 함수 */
void AFruitGameMode::PlayerRequestsStartTurn(AController* PlayerController)
{
	// 돌림판 단계가 아니거나 결과가 아직 없으면 무시
	if (!MyGameState || MyGameState->CurrentGamePhase != EGamePhase::GP_SpinnerTurn || SpinnerResultIndex == -1)
	{
		return;
	}

	// (선택 사항) 이미 턴이 시작되었는지 확인하여 중복 호출 방지
	// (단, 2명이 동시에 호출할 수 있으므로 첫 호출자만 처리)
	if (MyGameState->CurrentGamePhase == EGamePhase::GP_PlayerTurn)
	{
		return;
	}

	// SpinnerResultIndex를 사용하여 첫 턴 플레이어 설정
	MyGameState->CurrentActivePlayer = MyGameState->PlayerArray[SpinnerResultIndex];

	// (중요!) GameState의 Phase를 PlayerTurn으로 변경
	MyGameState->CurrentGamePhase = EGamePhase::GP_PlayerTurn;

	// 첫 턴 시작
	StartTurn();
}


// --- 4. PlayerTurn 단계 ---
// (StartTurn, OnTurnTimerExpired, IsPlayerTurn, ProcessGuessFromWorldObjects, ProcessPlayerGuess, EndTurn, EndGame 함수는 이전 버전과 동일하게 유지)

void AFruitGameMode::StartTurn()
{
	if (!MyGameState || !MyGameState->CurrentActivePlayer) return;
	MyGameState->ServerTimeAtTurnStart = GetWorld()->GetTimeSeconds();
	GetWorldTimerManager().SetTimer(TurnTimerHandle, this, &AFruitGameMode::OnTurnTimerExpired, TurnDuration, false);
	AFruitPlayerController* ActivePC = Cast<AFruitPlayerController>(MyGameState->CurrentActivePlayer->GetPlayerController());
	if (ActivePC)
	{
		ActivePC->Client_StartTurn();
	}
}

void AFruitGameMode::OnTurnTimerExpired()
{
	EndTurn(true);
}

bool AFruitGameMode::IsPlayerTurn(AController* PlayerController) const
{
	if (!MyGameState || !PlayerController || !PlayerController->PlayerState)
	{
		return false;
	}
	return (MyGameState->CurrentActivePlayer == PlayerController->PlayerState);
}

void AFruitGameMode::ProcessGuessFromWorldObjects(AController* PlayerController)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableFruitObject::StaticClass(), FoundActors);
	if (FoundActors.Num() != 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessGuessFromWorldObjects: 5개의 추측(Guessing) 오브젝트를 찾을 수 없습니다."));
		return;
	}
	FoundActors.Sort([](const AActor& A, const AActor& B) {
		const AInteractableFruitObject* ObjA = Cast<AInteractableFruitObject>(&A);
		const AInteractableFruitObject* ObjB = Cast<AInteractableFruitObject>(&B);
		if (ObjA && ObjB) return ObjA->GuessIndex < ObjB->GuessIndex;
		return false;
		});
	TArray<EFruitType> GuessedFruits;
	GuessedFruits.Init(EFruitType::FT_None, 5);
	bool bAllValid = true;
	for (AActor* Actor : FoundActors)
	{
		AInteractableFruitObject* FruitObject = Cast<AInteractableFruitObject>(Actor);
		if (FruitObject && GuessedFruits.IsValidIndex(FruitObject->GuessIndex))
		{
			GuessedFruits[FruitObject->GuessIndex] = FruitObject->CurrentFruit;
		}
		else
		{
			bAllValid = false;
		}
	}
	if (bAllValid)
	{
		ProcessPlayerGuess(PlayerController, GuessedFruits);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessGuessFromWorldObjects: 추측(Guessing) 오브젝트의 GuessIndex가 잘못 설정되었습니다. (0-4)"));
	}
}

void AFruitGameMode::ProcessPlayerGuess(AController* PlayerController, const TArray<EFruitType>& GuessedFruits)
{
	if (!IsPlayerTurn(PlayerController))
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	if (MyGameState) MyGameState->ServerTimeAtTurnStart = 0.0f;
	AFruitPlayerState* OpponentPS = nullptr;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		if (PS != PlayerController->PlayerState)
		{
			OpponentPS = Cast<AFruitPlayerState>(PS);
			break;
		}
	}
	if (!OpponentPS) return;
	const TArray<EFruitType>& OpponentSecret = OpponentPS->GetSecretAnswers_Server();
	int32 MatchCount = 0;
	for (int32 i = 0; i < 5; ++i)
	{
		if (GuessedFruits.IsValidIndex(i) && OpponentSecret.IsValidIndex(i) &&
			GuessedFruits[i] == OpponentSecret[i] && GuessedFruits[i] != EFruitType::FT_None)
		{
			MatchCount++;
		}
	}
	AFruitPlayerController* GuesserPC = Cast<AFruitPlayerController>(PlayerController);
	AFruitPlayerController* OpponentPC = Cast<AFruitPlayerController>(OpponentPS->GetPlayerController());
	if (GuesserPC)
	{
		GuesserPC->Client_ReceiveGuessResult(GuessedFruits, MatchCount);
	}
	if (OpponentPC)
	{
		OpponentPC->Client_OpponentGuessed(GuessedFruits, MatchCount);
	}
	if (MatchCount == 5)
	{
		EndGame(PlayerController->PlayerState);
	}
	else
	{
		EndTurn(false);
	}
}

void AFruitGameMode::EndTurn(bool bTimeOut)
{
	if (!MyGameState) return;
	if (bTimeOut)
	{
		if (MyGameState) MyGameState->ServerTimeAtTurnStart = 0.0f;
	}
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
		StartTurn();
	}
	else
	{
		EndGame(nullptr);
	}
}

void AFruitGameMode::EndGame(APlayerState* Winner)
{
	if (!MyGameState) return;
	MyGameState->CurrentGamePhase = EGamePhase::GP_GameOver;
	MyGameState->Winner = Winner;
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	if (MyGameState) MyGameState->ServerTimeAtTurnStart = 0.0f;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
		if (PC)
		{
			PC->Client_GameOver(PS == Winner);
		}
	}
}