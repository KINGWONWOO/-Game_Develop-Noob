// FruitGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FruitGameTypes.h"
#include "FruitGameMode.generated.h"

class AFruitGameState;
class AFruitPlayerState;
class AFruitPlayerController;

UCLASS()
class NOOBGAME_API AFruitGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFruitGameMode();

	// --- PlayerController가 호출하는 함수 ---
	void PlayerSubmittedFruits(AController* PlayerController, const TArray<EFruitType>& SecretFruits);
	void ProcessPlayerGuess(AController* PlayerController, const TArray<EFruitType>& GuessedFruits);

	// 현재 턴인 플레이어가 맞는지 확인
	bool IsPlayerTurn(AController* PlayerController) const;

protected:
	// 플레이어가 레벨에 접속 완료 시 호출
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 게임 틱 (타이머 감소용)
	virtual void Tick(float DeltaSeconds) override;

	// --- 게임 흐름 제어 함수 ---

	// 2명이 모두 정답을 제출했는지 확인
	void CheckBothPlayersReady();

	// 동전 던지기 및 첫 턴 시작
	void StartCoinToss();

	// 턴 시작 (타이머 설정)
	void StartTurn();

	// 턴 종료 (타임아웃 또는 추측 완료)
	void EndTurn(bool bTimeOut);

	// 게임 종료 (승자 결정)
	void EndGame(APlayerState* Winner);

	// 턴 시간 만료 시 호출될 함수
	void OnTurnTimerExpired();

	// 캐시된 GameState
	UPROPERTY()
	AFruitGameState* MyGameState;

	// 턴 타이머 핸들
	FTimerHandle TurnTimerHandle;

	// 턴당 제한 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	float TurnDuration = 30.0f;
};