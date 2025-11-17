#include "FruitGame/FruitGameMode.h"
#include "FruitGame/FruitGameState.h"
#include "FruitGame/FruitPlayerState.h"
#include "FruitGame/FruitPlayerController.h"
#include "GameFramework/Controller.h"
#include "FruitGame/InteractableFruitObject.h"
#include "FruitGame/SubmitGuessButton.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "NoobGame/NoobGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/TargetPoint.h"
#include "Camera/CameraActor.h" // CameraActor 헤더 (EndGame에서 사용)

AFruitGameMode::AFruitGameMode()
{
	GameStateClass = AFruitGameState::StaticClass();
	PlayerStateClass = AFruitPlayerState::StaticClass();
	PlayerControllerClass = AFruitPlayerController::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
	MyGameState = nullptr;
	NumPlayersReady_Setup = 0;
	SpinnerResultIndex = -1;
}

// 로그인 관리
void AFruitGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!MyGameState)
	{
		MyGameState = GetGameState<AFruitGameState>();
	}

	if (NewPlayer && MyGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] PostLogin: Player %s joined. Total Players: %d. Current Phase: %s"),
			*NewPlayer->GetName(), GetNumPlayers(), *UEnum::GetValueAsString(MyGameState->CurrentGamePhase));
	}

	if (MyGameState && GetNumPlayers() == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] PostLogin: 2 players detected. Changing phase to GP_Instructions."));
		MyGameState->CurrentGamePhase = EFruitGamePhase::GP_Instructions;
	}
}

void AFruitGameMode::OnTurnTimerExpired()
{
	EndTurn(true);
}

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

void AFruitGameMode::PlayerInteracted(AController* PlayerController, AActor* HitActor, EFruitGamePhase CurrentPhase)
{
	if (CurrentPhase == EFruitGamePhase::GP_PlayerTurn)
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
	if (!MyGameState || MyGameState->CurrentGamePhase != EFruitGamePhase::GP_Instructions) return;
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
		UE_LOG(LogTemp, Warning, TEXT("[Server] ... 2명 모두 준비됨. 페이즈를 GP_Setup로 변경합니다."));
		MyGameState->CurrentGamePhase = EFruitGamePhase::GP_Setup;
	}
}

// --- 2. Setup 단계 ---
void AFruitGameMode::PlayerSubmittedFruits(AController* PlayerController, const TArray<EFruitType>& SecretFruits)
{
	if (!MyGameState || MyGameState->CurrentGamePhase != EFruitGamePhase::GP_Setup) return;
	AFruitPlayerState* PS = PlayerController->GetPlayerState<AFruitPlayerState>();
	if (PS && !PS->bHasSubmittedFruits)
	{
		PS->SetSecretAnswers_Server(SecretFruits);
		NumPlayersReady_Setup++;
		CheckBothPlayersReady_Setup();
	}
}

void AFruitGameMode::CheckBothPlayersReady_Setup()
{

	if (NumPlayersReady_Setup == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] ... 2명 모두 제출함. 페이즈를 GP_SpinnerTurn로 변경합니다."));
		StartSpinnerPhase();
	}
}

// --- 3. SpinnerTurn 단계 ---
void AFruitGameMode::StartSpinnerPhase()
{
	if (!MyGameState) return;
	MyGameState->CurrentGamePhase = EFruitGamePhase::GP_SpinnerTurn;
	SpinnerResultIndex = FMath::RandRange(0, 1);
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
		if (PC)
		{
			PC->Client_PlaySpinnerAnimation(SpinnerResultIndex);
		}
	}
}

void AFruitGameMode::PlayerRequestsStartTurn(AController* PlayerController)
{
	if (!MyGameState || MyGameState->CurrentGamePhase != EFruitGamePhase::GP_SpinnerTurn || SpinnerResultIndex == -1) return;
	if (MyGameState->CurrentGamePhase == EFruitGamePhase::GP_PlayerTurn) return;

	MyGameState->CurrentActivePlayer = MyGameState->PlayerArray[SpinnerResultIndex];
	MyGameState->CurrentGamePhase = EFruitGamePhase::GP_PlayerTurn;
	StartTurn();
}

bool AFruitGameMode::IsPlayerTurn(AController* PlayerController) const
{
	if (!MyGameState || !PlayerController || !PlayerController->PlayerState) return false;
	return (MyGameState->CurrentActivePlayer == PlayerController->PlayerState);
}

void AFruitGameMode::ProcessGuessFromWorldObjects(AController* PlayerController)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableFruitObject::StaticClass(), FoundActors);
	if (FoundActors.Num() != 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessGuessFromWorldObjects: 5 guessing objects not found."));
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
		UE_LOG(LogTemp, Error, TEXT("ProcessGuessFromWorldObjects: Guessing object GuessIndex is invalid (0-4)."));
	}
}

// --- [대폭 수정됨] ProcessPlayerGuess는 딜레이 로직을 포함합니다 ---
void AFruitGameMode::ProcessPlayerGuess(AController* PlayerController, const TArray<EFruitType>& GuessedFruits)
{
	if (!IsPlayerTurn(PlayerController)) return;

	// 턴 타이머(30초)를 즉시 정지시킵니다.
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

	// 정답 비교
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

	// 결과를 UI에 즉시 표시하기 위해 RPC를 먼저 호출합니다.
	AFruitPlayerController* GuesserPC = Cast<AFruitPlayerController>(PlayerController);
	AFruitPlayerController* OpponentPC = Cast<AFruitPlayerController>(OpponentPS->GetPlayerController());
	if (GuesserPC)
	{
		// 추측한 사람에게 "N개 맞음" 또는 "승리!"를 알림
		GuesserPC->Client_ReceiveGuessResult(GuessedFruits, MatchCount);
	}
	if (OpponentPC)
	{
		// 상대방에게 "상대가 N개 맞춤" 또는 "패배!"를 알림
		OpponentPC->Client_OpponentGuessed(GuessedFruits, MatchCount);
	}

	if (MatchCount == 5)
	{
		GetWorldTimerManager().SetTimer(
			EndGameDelayTimerHandle,
			[this, PlayerState = PlayerController->PlayerState]()
			{
				EndGame(PlayerState);
			},
			3.0f,
			false
		);
	}
	else
	{
		// [기존] 정답이 아닐 경우: 3초 딜레이 후 EndTurn 호출
		GetWorldTimerManager().SetTimer(
			GuessResultTimerHandle,
			this,
			&AFruitGameMode::OnGuessResultDelayExpired,
			GuessResultDisplayTime,
			false
		);
	}
}

/** (오답 시) 추측 결과 UI 딜레이가 끝났을 때 호출되는 함수 */
void AFruitGameMode::OnGuessResultDelayExpired()
{
	// 3초가 지났으므로 다음 턴으로 넘깁니다.
	EndTurn(false);
}


void AFruitGameMode::EndTurn(bool bTimeOut)
{
	if (!MyGameState) return;
	if (bTimeOut)
	{
		if (MyGameState) MyGameState->ServerTimeAtTurnStart = 0.0f;
	}

	// [수정] 모든 딜레이 타이머를 클리어합니다.
	GetWorldTimerManager().ClearTimer(GuessResultTimerHandle);
	GetWorldTimerManager().ClearTimer(EndGameDelayTimerHandle);

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
		EndGame(nullptr); // 비기는 경우
	}
}


// --- ProcessPunch ---
void AFruitGameMode::ProcessPunch(APlayerController* PuncherController, ACharacter* HitCharacter)
{
	if (!HitCharacter || !HitCharacter->GetController() || !PuncherController || !PuncherController->GetPawn()) return;

	AFruitPlayerState* HitPlayerState = HitCharacter->GetController()->GetPlayerState<AFruitPlayerState>();

	// 이미 K.O. 상태이거나 게임이 끝났으면 무시
	if (!MyGameState || !HitPlayerState || HitPlayerState->bIsKnockedDown || MyGameState->CurrentGamePhase == EFruitGamePhase::GP_GameOver) return;

	const FVector PunchDirection = (HitCharacter->GetActorLocation() - PuncherController->GetPawn()->GetActorLocation()).GetSafeNormal();
	HitCharacter->GetCharacterMovement()->AddImpulse(PunchDirection * PunchPushForce, true);

	HitPlayerState->PunchHitCount++;

	if (HitPlayerState->PunchHitCount >= 10)
	{
		// K.O. 상태 시작
		HitPlayerState->bIsKnockedDown = true;
		HitPlayerState->PunchHitCount = 0;

		ANoobGameCharacter* HitChar = Cast<ANoobGameCharacter>(HitCharacter);
		if (HitChar)
		{
			HitChar->SetRagdollState_Server(true); // 래그돌 활성화
		}

		if (AFruitPlayerController* HitPC = Cast<AFruitPlayerController>(HitCharacter->GetController()))
		{
			HitPC->Client_SetCameraEffect(true); // 카메라 효과 활성화
		}

		// 4초 뒤 RecoverCharacter를 호출하는 타이머 설정
		FTimerHandle KnockdownTimer;
		FTimerDelegate TimerDel = FTimerDelegate::CreateUObject(this, &AFruitGameMode::RecoverCharacter, HitCharacter);
		GetWorldTimerManager().SetTimer(KnockdownTimer, TimerDel, KnockdownDuration, false);

		// 다중 K.O.를 대비해 타이머 핸들 저장
		KnockdownTimers.Add(HitCharacter, KnockdownTimer);
	}
	else
	{
		// 10대 미만 피격 반응
		ANoobGameCharacter* HitChar = Cast<ANoobGameCharacter>(HitCharacter);
		if (!HitChar) return;

		// 맞는 방향을 계산하여 적절한 몽타주 선택
		const FVector HitVector = (HitCharacter->GetActorLocation() - PuncherController->GetPawn()->GetActorLocation()).GetSafeNormal();
		const FVector ActorForward = HitChar->GetActorForwardVector();
		const FVector ActorRight = HitChar->GetActorRightVector();
		float ForwardDot = FVector::DotProduct(HitVector, ActorForward);
		float RightDot = FVector::DotProduct(HitVector, ActorRight);

		UAnimMontage* SelectedMontage = nullptr;
		if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
		{
			SelectedMontage = (ForwardDot > 0) ? HitChar->HitReaction_Back : HitChar->HitReaction_Front;
		}
		else
		{
			SelectedMontage = (RightDot > 0) ? HitChar->HitReaction_Right : HitChar->HitReaction_Left;
		}
		if (!SelectedMontage)
		{
			SelectedMontage = HitChar->HitReaction_Front;
		}
		if (MyGameState && SelectedMontage)
		{
			for (APlayerState* PS : MyGameState->PlayerArray)
			{
				AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
				if (PC)
				{
					PC->Multicast_PlayHitReaction(HitCharacter, SelectedMontage);
				}
			}
		}
	}
}


void AFruitGameMode::EndGame(APlayerState* Winner)
{
	if (!MyGameState || MyGameState->CurrentGamePhase == EFruitGamePhase::GP_GameOver) return;

	UE_LOG(LogTemp, Warning, TEXT("[Server GM] EndGame Started. Winner: %s"), Winner ? *Winner->GetPlayerName() : TEXT("None"));

	MyGameState->CurrentGamePhase = EFruitGamePhase::GP_GameOver;
	MyGameState->Winner = Winner;

	// [수정] 모든 딜레이 타이머를 확실히 클리어합니다.
	GetWorldTimerManager().ClearTimer(TurnTimerHandle);
	GetWorldTimerManager().ClearTimer(GuessResultTimerHandle);
	GetWorldTimerManager().ClearTimer(EndGameDelayTimerHandle);

	MyGameState->ServerTimeAtTurnStart = 0.0f;

	// 진행 중이던 모든 K.O. 타이머를 강제로 클리어
	for (auto& TimerPair : KnockdownTimers)
	{
		GetWorldTimerManager().ClearTimer(TimerPair.Value);
	}
	KnockdownTimers.Empty();

	// --- 1. 승자와 패자 PlayerState 찾기 ---
	APlayerState* Loser = nullptr;
	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		if (PS && PS != Winner)
		{
			Loser = PS;
			break;
		}
	}
	if (!Winner || !Loser)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server GM] EndGame: Could not find Winner or Loser!"), MyGameState->PlayerArray.Num());
		return;
	}

	// --- 2. 스폰 지점 및 카메라 찾기 ---
	TArray<AActor*> WinnerSpawns;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), FName("Result_Spawn_Winner"), WinnerSpawns);
	AActor* WinnerSpawnPoint = (WinnerSpawns.Num() > 0) ? WinnerSpawns[0] : nullptr;

	TArray<AActor*> DefeatSpawns;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), FName("Result_Spawn_Defeat"), DefeatSpawns);
	AActor* DefeatSpawnPoint = (DefeatSpawns.Num() > 0) ? DefeatSpawns[0] : nullptr;

	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("EndingCamera"), CameraActors);
	ACameraActor* EndingCamera = (CameraActors.Num() > 0) ? Cast<ACameraActor>(CameraActors[0]) : nullptr;


	if (!WinnerSpawnPoint || !DefeatSpawnPoint || !EndingCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server GM] EndGame: FAILED to find TargetPoints or EndingCamera!"));
	}

	// --- 3. 승자 캐릭터 타입 설정 (GameState용) ---
	AFruitPlayerController* WinnerPC = Cast<AFruitPlayerController>(Winner->GetPlayerController());
	ANoobGameCharacter* WinnerPawn = WinnerPC ? Cast<ANoobGameCharacter>(WinnerPC->GetPawn()) : nullptr;
	if (WinnerPawn)
	{
		if (WinnerPawn->ActorHasTag(FName("Cat"))) MyGameState->WinningCharacterType = ECharacterType::ECT_Cat;
		else if (WinnerPawn->ActorHasTag(FName("Dog"))) MyGameState->WinningCharacterType = ECharacterType::ECT_Dog;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Server GM] EndGame: WinnerType set to %s"), *UEnum::GetValueAsString(MyGameState->WinningCharacterType));

	// --- 4. 각 PlayerController에게 [서버에서] 엔딩 처리를 위임 ---
	AFruitPlayerController* LoserPC = Cast<AFruitPlayerController>(Loser->GetPlayerController());

	if (WinnerPC && WinnerSpawnPoint && EndingCamera)
	{
		WinnerPC->Server_SetupEnding(true, WinnerSpawnPoint->GetActorLocation(), WinnerSpawnPoint->GetActorRotation(), MyGameState->WinningCharacterType, EndingCamera);
	}

	if (LoserPC && DefeatSpawnPoint && EndingCamera)
	{
		LoserPC->Server_SetupEnding(false, DefeatSpawnPoint->GetActorLocation(), DefeatSpawnPoint->GetActorRotation(), MyGameState->WinningCharacterType, EndingCamera);
	}

	// 5초 타이머 설정 제거 ---
	UE_LOG(LogTemp, Warning, TEXT("[Server GM] EndGame Finished. (No movement lock)"));
}

// --- 5. 펀치 애니메이션 ---
void AFruitGameMode::ProcessPunchAnimation(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay)
{
	if (!MyGameState || !PunchingCharacter || !MontageToPlay) return;

	for (APlayerState* PS : MyGameState->PlayerArray)
	{
		AFruitPlayerController* PC = Cast<AFruitPlayerController>(PS->GetPlayerController());
		if (PC)
		{
			PC->Multicast_PlayPunchMontage(PunchingCharacter, MontageToPlay);
		}
	}
}

// --- K.O.에서 회복시키는 함수 ---
void AFruitGameMode::RecoverCharacter(ACharacter* CharacterToRecover)
{
	if (!CharacterToRecover || !CharacterToRecover->GetController()) return;

	// 타이머 핸들 맵에서 제거
	KnockdownTimers.Remove(CharacterToRecover);

	// 게임이 그 사이에 끝났다면 아무것도 하지 않음 (EndGame이 이미 래그돌을 풀었을 것임)
	if (MyGameState && MyGameState->CurrentGamePhase == EFruitGamePhase::GP_GameOver)
	{
		return;
	}

	AFruitPlayerState* PS = CharacterToRecover->GetController()->GetPlayerState<AFruitPlayerState>();
	if (PS && PS->bIsKnockedDown)
	{
		PS->bIsKnockedDown = false; // K.O. 상태 해제

		ANoobGameCharacter* RecoverChar = Cast<ANoobGameCharacter>(CharacterToRecover);
		if (RecoverChar)
		{
			RecoverChar->SetRagdollState_Server(false); // 래그돌 해제
		}

		if (AFruitPlayerController* RecoverPC = Cast<AFruitPlayerController>(CharacterToRecover->GetController()))
		{
			RecoverPC->Client_SetCameraEffect(false); // 카메라 효과 해제
		}
	}
}