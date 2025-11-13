// OXQuizGameMode.cpp

#include "OXQuizGameMode.h"
#include "QuizGame/OXQuizGameState.h"
#include "QuizGame/QuizObstacleBase.h"
#include "NoobGameCharacter.h" // [수정] 기존 캐릭터 헤더 포함
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"

AOXQuizGameMode::AOXQuizGameMode()
{
    GameStateClass = AOXQuizGameState::StaticClass();
    DefaultPawnClass = ANoobGameCharacter::StaticClass(); // [수정]

    // ... (나머지 설정은 이전과 동일) ...
    ReadyPhaseDuration = 5.0f;
    InitialSpawnDelay = 3.0f;
    TimeBetweenSpawns = 10.0f;
    MinMoveSpeed = 200.0f;
    MaxMoveSpeed = 800.0f;
    SpeedIncrement = 20.0f;
}

// ... (BeginPlay, PostLogin, Logout, SetGamePhase, OnPhaseChanged 등 모든 함수는 이전과 동일) ...

void AOXQuizGameMode::BeginPlay()
{
    Super::BeginPlay();
    CachedGameState = GetGameState<AOXQuizGameState>();
    if (CachedGameState)
    {
        SetGamePhase(EQuizGamePhase::WaitingToStart);
    }
}

void AOXQuizGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (GetNumPlayers() >= 2 && CachedGameState && CachedGameState->GetCurrentPhase() == EQuizGamePhase::WaitingToStart)
    {
        SetGamePhase(EQuizGamePhase::Ready);
    }
}

void AOXQuizGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    if (CachedGameState && (CachedGameState->GetCurrentPhase() == EQuizGamePhase::Playing || CachedGameState->GetCurrentPhase() == EQuizGamePhase::Ready))
    {
        if (GetNumPlayers() <= 1)
        {
            HandlePlayerDeath(Exiting);
        }
    }
}

void AOXQuizGameMode::SetGamePhase(EQuizGamePhase NewPhase)
{
    if (!CachedGameState || CachedGameState->GetCurrentPhase() == NewPhase)
    {
        return;
    }
    CachedGameState->SetCurrentPhase(NewPhase);
    OnPhaseChanged(NewPhase);
}

void AOXQuizGameMode::OnPhaseChanged(EQuizGamePhase NewPhase)
{
    GetWorldTimerManager().ClearTimer(TimerHandle_GamePhase);

    switch (NewPhase)
    {
    case EQuizGamePhase::Ready:
        StartReadyPhase();
        break;
    case EQuizGamePhase::Playing:
        StartPlayingPhase();
        break;
    case EQuizGamePhase::GameOver:
        StartGameOverPhase();
        break;
    default:
        break;
    }
}

void AOXQuizGameMode::StartReadyPhase()
{
    GetWorldTimerManager().SetTimer(
        TimerHandle_GamePhase,
        [this]() { SetGamePhase(EQuizGamePhase::Playing); },
        ReadyPhaseDuration,
        false
    );
}

void AOXQuizGameMode::StartPlayingPhase()
{
    RemainingQuizList = FullQuizList;
    if (RemainingQuizList.Num() == 0)
    {
        UE_LOG(LogGameMode, Error, TEXT("FullQuizList가 비어있습니다!"));
        SetGamePhase(EQuizGamePhase::GameOver);
        return;
    }
    CurrentMoveSpeed = MinMoveSpeed;
    GetWorldTimerManager().SetTimer(TimerHandle_SpawnQuiz, this, &AOXQuizGameMode::SpawnNextQuizObstacle, InitialSpawnDelay, false);
}

void AOXQuizGameMode::StartGameOverPhase()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_SpawnQuiz);
    GetWorldTimerManager().ClearTimer(TimerHandle_GamePhase);

    TArray<AActor*> FoundObstacles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuizObstacleBase::StaticClass(), FoundObstacles);
    for (AActor* Actor : FoundObstacles)
    {
        Actor->Destroy();
    }
}

void AOXQuizGameMode::SpawnNextQuizObstacle()
{
    if (!CachedGameState || CachedGameState->GetCurrentPhase() != EQuizGamePhase::Playing)
    {
        return;
    }

    FQuizData SelectedQuiz;
    if (GetRandomQuiz(SelectedQuiz))
    {
        TSubclassOf<AQuizObstacleBase> ObstacleClassToSpawn = nullptr;
        if (SelectedQuiz.Answers.Num() == 2)
        {
            ObstacleClassToSpawn = QuizObstacleClass_2Choice;
        }
        else if (SelectedQuiz.Answers.Num() == 3)
        {
            ObstacleClassToSpawn = QuizObstacleClass_3Choice;
        }

        if (!ObstacleClassToSpawn)
        {
            UE_LOG(LogGameMode, Warning, TEXT("퀴즈 데이터(답 2/3개)에 맞는 장애물 클래스가 설정되지 않았습니다!"));
            GetWorldTimerManager().SetTimer(TimerHandle_SpawnQuiz, this, &AOXQuizGameMode::SpawnNextQuizObstacle, TimeBetweenSpawns, false);
            return;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AQuizObstacleBase* NewObstacle = GetWorld()->SpawnActor<AQuizObstacleBase>(
            ObstacleClassToSpawn,
            ObstacleSpawnTransform,
            SpawnParams
        );

        if (NewObstacle)
        {
            NewObstacle->InitializeObstacle(SelectedQuiz, CurrentMoveSpeed);
            CurrentMoveSpeed = FMath::Min(CurrentMoveSpeed + SpeedIncrement, MaxMoveSpeed);
            GetWorldTimerManager().SetTimer(TimerHandle_SpawnQuiz, this, &AOXQuizGameMode::SpawnNextQuizObstacle, TimeBetweenSpawns, false);
        }
    }
    else
    {
        UE_LOG(LogGameMode, Log, TEXT("모든 퀴즈 출제 완료. 퀴즈 목록을 리셋합니다."));
        RemainingQuizList = FullQuizList;
        GetWorldTimerManager().SetTimer(TimerHandle_SpawnQuiz, this, &AOXQuizGameMode::SpawnNextQuizObstacle, 0.1f, false);
    }
}

bool AOXQuizGameMode::GetRandomQuiz(FQuizData& OutQuiz)
{
    if (RemainingQuizList.Num() == 0)
    {
        return false;
    }
    int32 RandomIndex = FMath::RandRange(0, RemainingQuizList.Num() - 1);
    OutQuiz = RemainingQuizList[RandomIndex];
    RemainingQuizList.RemoveAt(RandomIndex);
    return true;
}

void AOXQuizGameMode::HandlePlayerDeath(AController* PlayerController)
{
    if (!CachedGameState || CachedGameState->GetCurrentPhase() == EQuizGamePhase::GameOver)
    {
        return;
    }

    APlayerState* LoserState = PlayerController ? PlayerController->GetPlayerState<APlayerState>() : nullptr;
    APlayerState* WinnerState = nullptr;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AController* OtherController = It->Get();
        if (OtherController && OtherController != PlayerController)
        {
            WinnerState = OtherController->GetPlayerState<APlayerState>();
            break;
        }
    }

    if (CachedGameState)
    {
        CachedGameState->Server_SetWinnerAndLoser(WinnerState, LoserState);
    }

    SetGamePhase(EQuizGamePhase::GameOver);
}