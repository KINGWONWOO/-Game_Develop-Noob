// OXQuizGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QuizGame/OXQuizTypes.h" // EQuizGamePhase를 알기 위해 포함
#include "OXQuizGameMode.generated.h"

class AOXQuizGameState;
class AQuizObstacleBase;

UCLASS()
class NOOBGAME_API AOXQuizGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOXQuizGameMode();

    /** DeadZoneVolume에서 호출됩니다. */
    void HandlePlayerDeath(AController* PlayerController);

protected:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    /** 게임 페이즈를 변경하고 타이머를 설정합니다. (GameState에도 복제) */
    void SetGamePhase(EQuizGamePhase NewPhase);

    /** Phase 변경 시 호출되는 내부 함수 */
    void OnPhaseChanged(EQuizGamePhase NewPhase);

    // ... (나머지 함수 선언은 이전과 동일) ...
    void StartReadyPhase();
    void StartPlayingPhase();
    void StartGameOverPhase();

    UFUNCTION()
    void SpawnNextQuizObstacle();
    bool GetRandomQuiz(FQuizData& OutQuiz);

    // ... (변수 선언은 이전과 동일) ...
    UPROPERTY()
    TObjectPtr<AOXQuizGameState> CachedGameState;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz")
    TArray<FQuizData> FullQuizList;

    UPROPERTY()
    TArray<FQuizData> RemainingQuizList;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz")
    TSubclassOf<AQuizObstacleBase> QuizObstacleClass_2Choice;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz")
    TSubclassOf<AQuizObstacleBase> QuizObstacleClass_3Choice;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz", meta = (MakeEditWidget = true))
    FTransform ObstacleSpawnTransform;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz|Timing")
    float ReadyPhaseDuration;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz|Timing")
    float InitialSpawnDelay;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz|Timing")
    float TimeBetweenSpawns;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz|Speed")
    float MinMoveSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz|Speed")
    float MaxMoveSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Quiz|Speed")
    float SpeedIncrement;

    UPROPERTY()
    float CurrentMoveSpeed;

    FTimerHandle TimerHandle_GamePhase;
    FTimerHandle TimerHandle_SpawnQuiz;
};