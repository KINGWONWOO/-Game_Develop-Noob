// OXQuizGameMode.cpp

#include "OXQuizGameMode.h"
#include "OXQuizGameState.h"
#include "OXQuizPlayerController.h"
#include "OXQuizPlayerState.h"
#include "QuizObstacleBase.h"
#include "NoobGameCharacter.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Engine/TargetPoint.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

AOXQuizGameMode::AOXQuizGameMode()
{
    GameStateClass = AOXQuizGameState::StaticClass();
    PlayerStateClass = AOXQuizPlayerState::StaticClass();
    PlayerControllerClass = AOXQuizPlayerController::StaticClass();
    DefaultPawnClass = ANoobGameCharacter::StaticClass();
    MyGameState = nullptr;

    SpeedLevels = { 400.f, 500.f, 600.f, 700.f, 800.f };
}

// 로그인 관리
void AOXQuizGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!MyGameState)
    {
        MyGameState = GetGameState<AOXQuizGameState>();
    }

    if (NewPlayer && MyGameState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Server] PostLogin: Player %s joined. Total Players: %d. Current Phase: %s"),
            *NewPlayer->GetName(), GetNumPlayers(), *UEnum::GetValueAsString(MyGameState->CurrentGamePhase));
    }

    if (MyGameState && GetNumPlayers() == 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Server] PostLogin: 2 players detected. Changing phase to GP_Instructions."));
        MyGameState->CurrentGamePhase = EQuizGamePhase::GP_Ready;
        for (APlayerState* PS : MyGameState->PlayerArray)
        {
            if (AOXQuizPlayerState* QPS = Cast<AOXQuizPlayerState>(PS))
            {
                QPS->bIsReady_Instructions = false;
                QPS->bIsKnockedDown = false;
                QPS->PunchHitCount = 0;
                QPS->bIsNextPunchLeft = true;
            }
        }
    }
}

void AOXQuizGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    if (MyGameState && (MyGameState->CurrentGamePhase == EQuizGamePhase::GP_Playing ||
        MyGameState->CurrentGamePhase == EQuizGamePhase::GP_Ready))
    {
        if (GetNumPlayers() <= 1) HandlePlayerDeath(Exiting);
    }
}

/* ────────────────────── Player Death / Ready ────────────────────── */
void AOXQuizGameMode::HandlePlayerDeath(AController* PlayerController)
{
    if (!MyGameState || MyGameState->CurrentGamePhase == EQuizGamePhase::GP_GameOver) return;

    APlayerState* LoserState = PlayerController ? PlayerController->GetPlayerState<APlayerState>() : nullptr;
    APlayerState* WinnerState = nullptr;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AController* Other = It->Get(); Other && Other != PlayerController)
        {
            WinnerState = Other->GetPlayerState<APlayerState>();
            break;
        }
    }

    // EndGame 호출
    EndGame(WinnerState);
}

void AOXQuizGameMode::PlayerIsReady(AController* PlayerController)
{
    if (!MyGameState || MyGameState->CurrentGamePhase != EQuizGamePhase::GP_Ready) return;

    if (AOXQuizPlayerState* PS = PlayerController->GetPlayerState<AOXQuizPlayerState>())
    {
        PS->SetInstructionReady_Server();
        CheckBothPlayersReady_Instruction();
    }
}

void AOXQuizGameMode::CheckBothPlayersReady_Instruction()
{
    if (!MyGameState) return;
    int32 ReadyCnt = 0;
    for (APlayerState* PS : MyGameState->PlayerArray)
        if (AOXQuizPlayerState* QPS = Cast<AOXQuizPlayerState>(PS))
            if (QPS->bIsReady_Instructions) ++ReadyCnt;

    if (ReadyCnt == 2) {
        MyGameState->CurrentGamePhase = EQuizGamePhase::GP_Playing;
        RemainingPlayingCountdown = PlayingStartCountdownDuration;
        if (MyGameState) MyGameState->SetPlayingCountdown(RemainingPlayingCountdown);

        GetWorldTimerManager().SetTimer(
            TimerHandle_GamePhase,
            this,
            &AOXQuizGameMode::UpdatePlayingCountdown,
            1.f,
            true);
    }
}

void AOXQuizGameMode::UpdatePlayingCountdown()
{
    --RemainingPlayingCountdown;
    if (MyGameState) MyGameState->SetPlayingCountdown(RemainingPlayingCountdown);
    if (RemainingPlayingCountdown <= 0)
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_GamePhase);
        StartQuizSpawning();
    }
}

/* ────────────────────── Quiz Spawning ────────────────────── */
void AOXQuizGameMode::StartQuizSpawning()
{
    if (!QuizDataTable) { UE_LOG(LogGameMode, Error, TEXT("QuizDataTable missing")); MyGameState->CurrentGamePhase = EQuizGamePhase::GP_GameOver; return; }

    TArray<FQuizData*> AllRows;
    QuizDataTable->GetAllRows(TEXT("StartQuizSpawning"), AllRows);
    RemainingQuizList.Empty();
    for (FQuizData* Row : AllRows) RemainingQuizList.Add(*Row);

    SpawnedQuizCount = 0;
    CurrentSpeedLevelIndex = 0;
    CurrentMoveSpeed = SpeedLevels.IsValidIndex(0) ? SpeedLevels[0] : 400.f;
    if (MyGameState) MyGameState->SetCurrentSpeedLevel(CurrentSpeedLevelIndex + 1);

    SpawnNextQuizObstacle();
}

void AOXQuizGameMode::SpawnNextQuizObstacle()
{
    if (!MyGameState || MyGameState->CurrentGamePhase != EQuizGamePhase::GP_Playing) return;

    FQuizData Quiz;
    if (GetRandomQuiz(Quiz))
    {
        TSubclassOf<AQuizObstacleBase> ObstacleClass = (Quiz.Answers.Num() == 2)
            ? QuizObstacleClass_2Choice
            : QuizObstacleClass_3Choice;

        if (!ObstacleClass) return;

        FActorSpawnParameters SP;
        SP.Owner = this;
        SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        if (AQuizObstacleBase* NewObs = GetWorld()->SpawnActor<AQuizObstacleBase>(ObstacleClass, ObstacleSpawnTransform, SP))
        {
            NewObs->InitializeObstacle(Quiz, CurrentMoveSpeed);
            ++SpawnedQuizCount;

            if (SpawnedQuizCount % 5 == 0 && SpeedLevels.IsValidIndex(++CurrentSpeedLevelIndex))
            {
                CurrentMoveSpeed = SpeedLevels[CurrentSpeedLevelIndex];
                if (MyGameState) MyGameState->SetCurrentSpeedLevel(CurrentSpeedLevelIndex + 1);
            }

            GetWorldTimerManager().SetTimer(TimerHandle_SpawnQuiz,
                this, &AOXQuizGameMode::SpawnNextQuizObstacle, TimeBetweenSpawns, false);
        }
    }
    else
    {
        RemainingQuizList.Empty();
        TArray<FQuizData*> AllRows;
        QuizDataTable->GetAllRows(TEXT("RespawnQuiz"), AllRows);
        for (FQuizData* Row : AllRows) RemainingQuizList.Add(*Row);
        GetWorldTimerManager().SetTimer(TimerHandle_SpawnQuiz,
            this, &AOXQuizGameMode::SpawnNextQuizObstacle, 0.1f, false);
    }
}

bool AOXQuizGameMode::GetRandomQuiz(FQuizData& OutQuiz)
{
    if (RemainingQuizList.IsEmpty()) return false;
    const int32 Idx = FMath::RandRange(0, RemainingQuizList.Num() - 1);
    OutQuiz = RemainingQuizList[Idx];
    RemainingQuizList.RemoveAt(Idx);
    return true;
}

/* ────────────────────── Combat (FruitGame) ────────────────────── */
void AOXQuizGameMode::ProcessPunch(APlayerController* PuncherController, ACharacter* HitCharacter)
{
    if (!HitCharacter || !HitCharacter->GetController() || !PuncherController || !PuncherController->GetPawn()) return;

    AOXQuizPlayerState* HitPlayerState = HitCharacter->GetController()->GetPlayerState<AOXQuizPlayerState>();

    // [수정] EQuizGamePhase 사용
    if (!MyGameState || !HitPlayerState || HitPlayerState->bIsKnockedDown || MyGameState->CurrentGamePhase == EQuizGamePhase::GP_GameOver) return;

    const FVector PunchDirection = (HitCharacter->GetActorLocation() - PuncherController->GetPawn()->GetActorLocation()).GetSafeNormal();
    HitCharacter->GetCharacterMovement()->AddImpulse(PunchDirection * PunchPushForce, true);

    HitPlayerState->PunchHitCount++;

    if (HitPlayerState->PunchHitCount >= 10)
    {
        // K.O.
        HitPlayerState->bIsKnockedDown = true;
        HitPlayerState->PunchHitCount = 0;

        ANoobGameCharacter* HitChar = Cast<ANoobGameCharacter>(HitCharacter);
        if (HitChar) HitChar->SetRagdollState_Server(true);

        if (AOXQuizPlayerController* HitPC = Cast<AOXQuizPlayerController>(HitCharacter->GetController()))
        {
            HitPC->Client_SetCameraEffect(true);
        }

        FTimerHandle KnockdownTimer;
        FTimerDelegate TimerDel = FTimerDelegate::CreateUObject(this, &AOXQuizGameMode::RecoverCharacter, HitCharacter);
        GetWorldTimerManager().SetTimer(KnockdownTimer, TimerDel, KnockdownDuration, false);
        KnockdownTimers.Add(HitCharacter, KnockdownTimer);
    }
    else
    {
        // Hit Reaction
        ANoobGameCharacter* HitChar = Cast<ANoobGameCharacter>(HitCharacter);
        if (!HitChar) return;

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
        if (!SelectedMontage) SelectedMontage = HitChar->HitReaction_Front;

        if (MyGameState && SelectedMontage)
        {
            for (APlayerState* PS : MyGameState->PlayerArray)
            {
                AOXQuizPlayerController* PC = Cast<AOXQuizPlayerController>(PS->GetPlayerController());
                if (PC)
                {
                    PC->Multicast_PlayHitReaction(HitCharacter, SelectedMontage);
                }
            }
        }
    }
}

void AOXQuizGameMode::RecoverCharacter(ACharacter* CharacterToRecover)
{
    if (!CharacterToRecover || !CharacterToRecover->GetController()) return;

    KnockdownTimers.Remove(CharacterToRecover);

    // [수정] EQuizGamePhase 사용
    if (MyGameState && MyGameState->CurrentGamePhase == EQuizGamePhase::GP_GameOver) return;

    AOXQuizPlayerState* PS = CharacterToRecover->GetController()->GetPlayerState<AOXQuizPlayerState>();
    if (PS && PS->bIsKnockedDown)
    {
        PS->bIsKnockedDown = false;
        ANoobGameCharacter* RecoverChar = Cast<ANoobGameCharacter>(CharacterToRecover);
        if (RecoverChar) RecoverChar->SetRagdollState_Server(false);

        if (AOXQuizPlayerController* RecoverPC = Cast<AOXQuizPlayerController>(CharacterToRecover->GetController()))
        {
            RecoverPC->Client_SetCameraEffect(false);
        }
    }
}

void AOXQuizGameMode::ProcessPunchAnimation(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay)
{
    if (!MyGameState || !PunchingCharacter || !MontageToPlay) return;

    for (APlayerState* PS : MyGameState->PlayerArray)
    {
        AOXQuizPlayerController* PC = Cast<AOXQuizPlayerController>(PS->GetPlayerController());
        if (PC)
        {
            PC->Multicast_PlayPunchMontage(PunchingCharacter, MontageToPlay);
        }
    }
}

/* ────────────────────── EndGame (FruitGame) ────────────────────── */
void AOXQuizGameMode::EndGame(APlayerState* Winner)
{
    // [수정] EQuizGamePhase 사용
    if (!MyGameState || MyGameState->CurrentGamePhase == EQuizGamePhase::GP_GameOver) return;

    UE_LOG(LogTemp, Warning, TEXT("[Server GM] EndGame Started. Winner: %s"), Winner ? *Winner->GetPlayerName() : TEXT("None"));

    // [수정] EQuizGamePhase 사용
    MyGameState->CurrentGamePhase = EQuizGamePhase::GP_GameOver;
    MyGameState->Winner = Winner;

    GetWorldTimerManager().ClearTimer(EndGameDelayTimerHandle);

    for (auto& Pair : KnockdownTimers)
    {
        GetWorldTimerManager().ClearTimer(Pair.Value);
    }
    KnockdownTimers.Empty();

    // Loser 찾기
    APlayerState* Loser = nullptr;
    for (APlayerState* PS : MyGameState->PlayerArray)
    {
        if (PS && PS != Winner)
        {
            Loser = PS;
            break;
        }
    }

    if (!Winner || !Loser) return;

    // 태그로 찾기
    TArray<AActor*> WinSpawns, LoseSpawns, CamActors;
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), FName("Result_Spawn_Winner"), WinSpawns);
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), FName("Result_Spawn_Defeat"), LoseSpawns);
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("EndingCamera"), CamActors);

    AActor* WinPoint = WinSpawns.IsValidIndex(0) ? WinSpawns[0] : nullptr;
    AActor* LosePoint = LoseSpawns.IsValidIndex(0) ? LoseSpawns[0] : nullptr;
    ACameraActor* EndCam = CamActors.IsValidIndex(0) ? Cast<ACameraActor>(CamActors[0]) : nullptr;

    if (!WinPoint || !LosePoint || !EndCam)
    {
        UE_LOG(LogTemp, Error, TEXT("[Server GM] EndGame: FAILED to find TargetPoints or EndingCamera!"));
    }

    // 승자 타입 설정 (FruitGame 로직)
    AOXQuizPlayerController* WinnerPC = Cast<AOXQuizPlayerController>(Winner->GetPlayerController());
    ANoobGameCharacter* WinnerPawn = WinnerPC ? Cast<ANoobGameCharacter>(WinnerPC->GetPawn()) : nullptr;
    
    if (WinnerPawn)
    {
        if (WinnerPawn->ActorHasTag(FName("Cat"))) MyGameState->WinningCharacterType = ECharacterType::ECT_Cat;
        else if (WinnerPawn->ActorHasTag(FName("Dog"))) MyGameState->WinningCharacterType = ECharacterType::ECT_Dog;
    }
    
    // OXQuiz에서는 일단 None 또는 특정 타입으로 설정
    ECharacterType WinType = ECharacterType::ECT_None;

    // 엔딩 처리 위임
    AOXQuizPlayerController* LoserPC = Cast<AOXQuizPlayerController>(Loser->GetPlayerController());

    if (WinnerPC && WinPoint && EndCam)
    {
        WinnerPC->Server_SetupEnding(true, WinPoint->GetActorLocation(), WinPoint->GetActorRotation(), WinType, EndCam);
    }

    if (LoserPC && LosePoint && EndCam)
    {
        LoserPC->Server_SetupEnding(false, LosePoint->GetActorLocation(), LosePoint->GetActorRotation(), WinType, EndCam);
    }

    // 장애물 파괴 (EndGame 마지막에)
    TArray<AActor*> Obstacles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuizObstacleBase::StaticClass(), Obstacles);
    for (AActor* A : Obstacles) A->Destroy();
}