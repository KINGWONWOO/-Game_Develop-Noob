// FruitGameMode.cpp

#include "FruitGame/FruitGameMode.h"
#include "FruitGame/FruitGameState.h"
#include "FruitGame/FruitPlayerState.h"
#include "FruitGame/FruitPlayerController.h"
#include "FruitGame/InteractableFruitObject.h"
#include "FruitGame/SubmitGuessButton.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h" 
#include "GameFramework/Character.h" 
#include "GameFramework/CharacterMovementComponent.h" 

AFruitGameMode::AFruitGameMode()
{
	// 기본 클래스 설정
	GameStateClass = AFruitGameState::StaticClass();
	PlayerStateClass = AFruitPlayerState::StaticClass();
	PlayerControllerClass = AFruitPlayerController::StaticClass();

	// 틱(Tick)은 사용하지 않음
	PrimaryActorTick.bCanEverTick = false;

	// 변수 초기화
	MyGameState = nullptr;
	NumPlayersReady_Setup = 0;
	SpinnerResultIndex = -1;

	// 펀치 기본값 설정
	PunchPushForce = 50000.0f;
	KnockdownDuration = 3.0f;
}

void AFruitGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// GameState 캐시
	if (!MyGameState)
	{
		MyGameState = GetGameState<AFruitGameState>();
	}

	// 2명 접속 시 Instructions 단계로 전환
	if (MyGameState && GetNumPlayers() == 2)
	{
		MyGameState->CurrentGamePhase = EGamePhase::GP_Instructions;
	}
}

// (GP_PlayerTurn 전용 상호작용)
void AFruitGameMode::PlayerInteracted(AController* PlayerController, AActor* HitActor, EGamePhase CurrentPhase)
{
	// 추측 턴(PlayerTurn)일 때만 작동
	if (CurrentPhase == EGamePhase::GP_PlayerTurn)
	{
		// 현재 턴인 플레이어인지 확인
		if (!IsPlayerTurn(PlayerController)) return;

		// 과일 오브젝트와 상호작용
		if (AInteractableFruitObject* GuessObject = Cast<AInteractableFruitObject>(HitActor))
		{
			GuessObject->CycleFruit(); // C++가 머티리얼 변경 자동 처리
		}
		// 추측 제출 버튼과 상호작용
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

/** Setup 완료 시 돌림판 단계 시작 */
void AFruitGameMode::CheckBothPlayersReady_Setup()
{
	if (NumPlayersReady_Setup == 2)
	{
		StartSpinnerPhase();
	}
}

// --- 3. SpinnerTurn 단계 ---

/** 돌림판 단계 시작 */
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
}

/** 블루프린트(UI)의 요청을 받아 실제 턴을 시작하는 함수 */
void AFruitGameMode::PlayerRequestsStartTurn(AController* PlayerController)
{
	if (!MyGameState || MyGameState->CurrentGamePhase != EGamePhase::GP_SpinnerTurn || SpinnerResultIndex == -1)
	{
		return;
	}

	if (MyGameState->CurrentGamePhase == EGamePhase::GP_PlayerTurn)
	{
		return;
	}

	MyGameState->CurrentActivePlayer = MyGameState->PlayerArray[SpinnerResultIndex];
	MyGameState->CurrentGamePhase = EGamePhase::GP_PlayerTurn;
	StartTurn();
}


// --- 4. PlayerTurn 단계 ---

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
	if (!IsPlayerTurn(PlayerController)) return;

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

// --- 5. 펀치 기능 ---

/** (신규!) 펀치 '애니메이션'을 모든 클라이언트에 재생하도록 지시 */
void AFruitGameMode::ProcessPunchAnimation(ACharacter* PunchingCharacter)
{
	if (!MyGameState || !PunchingCharacter) return;

	// 모든 PlayerController를 순회하며 각 PC에서 Multicast RPC를 호출
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
		if (PC)
		{
			// 이 PC의 Multicast RPC를 호출
			PC->Multicast_PlayPunchMontage(PunchingCharacter);
		}
	}
}

/** 펀치 '적중' 처리 함수 (서버에서만 실행됨) */
void AFruitGameMode::ProcessPunch(APlayerController* PuncherController, ACharacter* HitCharacter)
{
	if (!PuncherController || !PuncherController->GetPawn() || !HitCharacter || !HitCharacter->GetController()) return;

	AFruitPlayerState* HitPlayerState = HitCharacter->GetController()->GetPlayerState<AFruitPlayerState>();
	if (!HitPlayerState) return;

	if (HitPlayerState->bIsKnockedDown) return;

	// 1. 밀치기 효과 적용 (피격)
	FVector PunchDirection = (HitCharacter->GetActorLocation() - PuncherController->GetPawn()->GetActorLocation()).GetSafeNormal();
	PunchDirection.Z = 0.2f;
	HitCharacter->GetCharacterMovement()->AddImpulse(PunchDirection * PunchPushForce, true);

	// 2. 피격 횟수 증가
	HitPlayerState->PunchHitCount++;

	// 3. 쓰러짐 판정
	if (HitPlayerState->PunchHitCount >= 10)
	{
		HitPlayerState->bIsKnockedDown = true;
		HitPlayerState->PunchHitCount = 0;

		FTimerHandle RecoveryTimerHandle;
		FTimerDelegate RecoveryDelegate;
		RecoveryDelegate.BindUFunction(this, FName("RecoverCharacter"), HitCharacter);
		GetWorldTimerManager().SetTimer(RecoveryTimerHandle, RecoveryDelegate, KnockdownDuration, false);
	}
	else
	{
		// 1~9대째: 피격 애니메이션 전파
		if (MyGameState)
		{
			for (APlayerState* PS : MyGameState->PlayerArray)
			{
				AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
				if (PC)
				{
					PC->Multicast_PlayHitReaction(HitCharacter);
				}
			}
		}
	}
}

/** 캐릭터 회복 함수 (서버에서만 실행됨) */
void AFruitGameMode::RecoverCharacter(ACharacter* CharacterToRecover)
{
	if (!CharacterToRecover || !CharacterToRecover->GetController()) return;

	AFruitPlayerState* PS = CharacterToRecover->GetController()->GetPlayerState<AFruitPlayerState>();
	if (PS && PS->bIsKnockedDown)
	{
		PS->bIsKnockedDown = false;
	}
}