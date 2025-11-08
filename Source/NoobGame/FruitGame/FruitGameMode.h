#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FruitGame/FruitGameTypes.h"
#include "GameFramework/Pawn.h"
#include "FruitGameMode.generated.h"

class AFruitGameState;
class AFruitPlayerState;
class AFruitPlayerController;
class AActor;
class ACharacter;
class UAnimMontage;

UCLASS()
class NOOBGAME_API AFruitGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFruitGameMode();

	void PlayerIsReady(AController* PlayerController);
	void PlayerSubmittedFruits(AController* PlayerController, const TArray<EFruitType>& SecretFruits);
	void ProcessPlayerGuess(AController* PlayerController, const TArray<EFruitType>& GuessedFruits);
	void PlayerRequestsStartTurn(AController* PlayerController);
	bool IsPlayerTurn(AController* PlayerController) const;
	void PlayerInteracted(AController* PlayerController, AActor* HitActor, EGamePhase CurrentPhase);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ProcessPunchAnimation(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ProcessPunch(APlayerController* PuncherController, ACharacter* HitCharacter);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndGame(APlayerState* Winner);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 게임 흐름
	void CheckBothPlayersReady_Instructions();
	void CheckBothPlayersReady_Setup();
	void StartSpinnerPhase();
	void StartTurn();
	void EndTurn(bool bTimeOut);
	void OnTurnTimerExpired();
	void ProcessGuessFromWorldObjects(AController* PlayerController);

	UFUNCTION()
	void RecoverCharacter(ACharacter* CharacterToRecover);

	// [수정] 5초 타이머 함수 제거
	// void RestorePlayerControl(); 

	/** 호스트/Client 1이 사용할 폰 클래스 (BP_FirstPersonCharacter_Cat) */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerPawn")
	TSubclassOf<APawn> HostPawnClass;

	/** 참여자/Client 2가 사용할 폰 클래스 (BP_FirstPersonCharacter_Dog) */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerPawn")
	TSubclassOf<APawn> ClientPawnClass;

	UPROPERTY()
	AFruitGameState* MyGameState;

	FTimerHandle TurnTimerHandle;

	// [수정] 5초 타이머 핸들 제거
	// FTimerHandle TimerHandle_EndGameRestore;

	/** [신규] K.O. 상태인 플레이어와 복구 타이머를 매핑합니다. (TWeakObjectPtr로 안전하게) */
	TMap<TWeakObjectPtr<ACharacter>, FTimerHandle> KnockdownTimers;

	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	float TurnDuration = 30.0f;

	int32 NumPlayersReady_Setup = 0;

	UPROPERTY()
	int32 SpinnerResultIndex = -1;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float PunchPushForce = 50000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float KnockdownDuration = 4.0f; // 4초 K.O. 지속시간
};