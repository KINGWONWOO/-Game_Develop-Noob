#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameTypes.h"   // Common types (ECharacterType, etc.)
#include "GameFramework/Pawn.h"
#include "OXQuizGameMode.generated.h"

class AOXQuizGameState;
class AOXQuizPlayerState;
class AOXQuizPlayerController;
class AQuizObstacleBase;
class UDataTable;
class ANoobGameCharacter;
class AActor;
class ACharacter;
class UAnimMontage;

UCLASS()
class NOOBGAME_API AOXQuizGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOXQuizGameMode();

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Public Game Flow API (FruitGame Style)
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	/** [FruitGame] HandlePlayerReady -> PlayerIsReady 煎 滲唳 */
	void PlayerIsReady(AController* PlayerController);

	/** [OXQuiz Unique] Ы溯檜橫 驍塊 籀葬 */
	void HandlePlayerDeath(AController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndGame(APlayerState* Winner);

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Public Combat API (FruitGame Same)
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ProcessPunch(APlayerController* PuncherController, ACharacter* HitCharacter);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ProcessPunchAnimation(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay);

protected:
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Framework Overrides
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Internal Game Flow Logic
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	void CheckBothPlayersReady_Instruction(); // FruitGame style check

	// Phase Implementations
	void StartReadyPhase();
	void StartPlayingPhase();
	// StartGameOverPhase朝 EndGame戲煎 鱔м脾

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Internal Gameplay & Spawning (OXQuiz Specific)
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	UFUNCTION()
	void UpdatePlayingCountdown();

	UFUNCTION()
	void StartQuizSpawning();

	UFUNCTION()
	void SpawnNextQuizObstacle();

	bool GetRandomQuiz(FQuizData& OutQuiz);

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Internal Combat Logic
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	UFUNCTION()
	void RecoverCharacter(ACharacter* CharacterToRecover);

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Configuration Properties (Settings)
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式

	// -- Classes --
	/** �ˊ瘋�/Client 1檜 餌辨й ア 贗楚蝶 */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerPawn")
	TSubclassOf<APawn> HostPawnClass;

	/** 霤罹濠/Client 2陛 餌辨й ア 贗楚蝶 */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerPawn")
	TSubclassOf<APawn> ClientPawnClass;

	// -- Quiz Settings --
	UPROPERTY(EditDefaultsOnly, Category = "Quiz")
	TObjectPtr<UDataTable> QuizDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Quiz")
	TSubclassOf<AQuizObstacleBase> QuizObstacleClass_2Choice;

	UPROPERTY(EditDefaultsOnly, Category = "Quiz")
	TSubclassOf<AQuizObstacleBase> QuizObstacleClass_3Choice;

	UPROPERTY(EditDefaultsOnly, Category = "Quiz", meta = (MakeEditWidget = true))
	FTransform ObstacleSpawnTransform;

	// -- Timing / Rules --
	UPROPERTY(EditDefaultsOnly, Category = "Quiz|Timing")
	int32 PlayingStartCountdownDuration = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Quiz|Timing")
	float TimeBetweenSpawns = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Quiz|Speed")
	TArray<float> SpeedLevels;

	// -- Combat Settings --
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float PunchPushForce = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float KnockdownDuration = 4.f;

	// -- GameOver Settings --
	UPROPERTY(EditDefaultsOnly, Category = "Quiz|GameOver")
	FName WinnerSpawnTag = TEXT("Result_Spawn_Winner");

	UPROPERTY(EditDefaultsOnly, Category = "Quiz|GameOver")
	FName LoserSpawnTag = TEXT("Result_Spawn_Defeat");

	UPROPERTY(EditDefaultsOnly, Category = "Quiz|GameOver")
	FName EndingCameraTag = TEXT("EndingCamera");

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Runtime State & References
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	/** [FruitGame] CachedGameState 渠褐 MyGameState 餌辨 */
	UPROPERTY()
	AOXQuizGameState* MyGameState;


	// -- Timers --
	FTimerHandle EndGameDelayTimerHandle;
	FTimerHandle TimerHandle_GamePhase;
	FTimerHandle TimerHandle_SpawnQuiz;

	/** K.O. 犒掘 顫檜該 */
	UPROPERTY()
	TMap<TWeakObjectPtr<ACharacter>, FTimerHandle> KnockdownTimers;

	UPROPERTY()
	TArray<FQuizData> RemainingQuizList;

	// -- State Variables --
	UPROPERTY()
	float CurrentMoveSpeed = 0.f;

	UPROPERTY()
	int32 SpawnedQuizCount = 0;

	UPROPERTY()
	int32 CurrentSpeedLevelIndex = 0;

	UPROPERTY()
	int32 RemainingPlayingCountdown = 0;
};