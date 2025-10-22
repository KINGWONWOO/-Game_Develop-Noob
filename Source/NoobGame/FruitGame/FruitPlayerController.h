// FruitPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FruitGameTypes.h"
#include "FruitPlayerController.generated.h"

// UI 바인딩용 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuessResultReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOpponentGuessReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bYouWon);

UCLASS()
class NOOBGAME_API AFruitPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// --- UI에서 호출할 함수들 (블루프린트에서 호출 가능) ---

	// 1. (게임 준비 단계) UI에서 정답 과일 5개 선택 완료 시 호출
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	// 2. (내 턴) UI에서 추측 과일 5개 선택 완료 시 호출
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void SubmitGuess(const TArray<EFruitType>& GuessedFruits);

	// --- 서버 -> 클라이언트 RPC (서버가 클라이언트에서 함수를 호출) ---

	// 1. 서버가 "당신 턴입니다"라고 알림
	UFUNCTION(Client, Reliable)
	void Client_StartTurn();

	// 2. 서버가 "당신의 추측 결과입니다"라고 알림
	UFUNCTION(Client, Reliable)
	void Client_ReceiveGuessResult(const TArray<EFruitType>& Guess, int32 MatchCount);

	// 3. 서버가 "상대방의 추측 내용입니다"라고 알림
	UFUNCTION(Client, Reliable)
	void Client_OpponentGuessed(const TArray<EFruitType>& Guess, int32 MatchCount);

	// 4. 서버가 "게임 끝!"이라고 알림
	UFUNCTION(Client, Reliable)
	void Client_GameOver(bool bYouWon);

	// --- UI 바인딩용 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnTurnStarted OnTurnStarted;

	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnGuessResultReceived OnGuessResultReceived;

	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnOpponentGuessReceived OnOpponentGuessReceived;

	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnGameOver OnGameOver;

protected:
	// --- 클라이언트 -> 서버 RPC (클라이언트가 서버에서 함수를 호출) ---

	// SubmitSecretFruits의 실제 서버 실행 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	// SubmitGuess의 실제 서버 실행 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SubmitGuess(const TArray<EFruitType>& GuessedFruits);
};