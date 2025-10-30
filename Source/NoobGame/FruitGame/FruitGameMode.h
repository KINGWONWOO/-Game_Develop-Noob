// FruitGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FruitGame/FruitGameTypes.h"
#include "FruitGameMode.generated.h"

class AFruitGameState;
class AFruitPlayerState;
class AFruitPlayerController;
class AActor;

UCLASS()
class NOOBGAME_API AFruitGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFruitGameMode();

	// --- PlayerController가 호출하는 함수 ---

	/** 1. Instructions 단계에서 준비 완료 */
	void PlayerIsReady(AController* PlayerController);

	/** 2. Setup 단계에서 (UI로부터) 정답 제출 */
	void PlayerSubmittedFruits(AController* PlayerController, const TArray<EFruitType>& SecretFruits);

	/** 3. 턴 진행 중 (월드로부터) 추측 제출 */
	void ProcessPlayerGuess(AController* PlayerController, const TArray<EFruitType>& GuessedFruits);

	/** (신규!) 4. 블루프린트(PlayerController)에서 호출되어 실제 턴을 시작합니다. */
	void PlayerRequestsStartTurn(AController* PlayerController);

	/** 현재 턴인 플레이어가 맞는지 확인 */
	bool IsPlayerTurn(AController* PlayerController) const;

	/** (유지) PlayerController의 RPC가 호출하는 (GP_PlayerTurn 전용) 상호작용 핸들러 */
	void PlayerInteracted(AController* PlayerController, AActor* HitActor, EGamePhase CurrentPhase);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// (삭제) 틱 함수는 사용하지 않음
	// virtual void Tick(float DeltaSeconds) override;

	// --- 게임 흐름 제어 함수 ---
	void CheckBothPlayersReady_Instructions();

	/** (수정) Setup 완료 시 StartSpinnerPhase를 호출합니다. */
	void CheckBothPlayersReady_Setup();

	/** (신규) 돌림판 단계를 시작하고, 결과를 미리 결정합니다. */
	void StartSpinnerPhase();

	// (삭제) 타이머로 자동 시작하던 함수들 삭제
	// void OnSpinnerAnimationFinished();
	// void DetermineFirstPlayerAndStartTurn();

	void StartTurn();
	void EndTurn(bool bTimeOut);
	void EndGame(APlayerState* Winner);
	void OnTurnTimerExpired();

	/** 추측 턴에 월드 오브젝트로부터 추측 배열을 생성하고 제출합니다. */
	void ProcessGuessFromWorldObjects(AController* PlayerController);

	/** 캐시된 GameState */
	UPROPERTY()
	AFruitGameState* MyGameState;

	/** 턴 타이머 핸들 (GP_PlayerTurn용) */
	FTimerHandle TurnTimerHandle;

	/** 턴당 제한 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	float TurnDuration = 30.0f;

	/** Setup 준비 완료 인원 */
	int32 NumPlayersReady_Setup = 0;

	// (삭제) 돌림판 애니메이션용 타이머 핸들 삭제
	// FTimerHandle SpinnerTimerHandle;

	/** (유지) 서버에서 결정된 돌림판 결과 (0 또는 1) */
	UPROPERTY()
	int32 SpinnerResultIndex = -1;
};