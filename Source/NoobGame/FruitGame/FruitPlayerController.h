// FruitPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FruitGame/FruitGameTypes.h"
#include "FruitPlayerController.generated.h"

// --- 델리게이트 선언 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuessResultReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOpponentGuessReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bYouWon);

// --- 전방 선언 ---
class AInteractableFruitObject;

UCLASS()
class NOOBGAME_API AFruitPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	// 로컬 정답 저장용 변수
	TArray<EFruitType> MyLocalSecretAnswers;

public:
	// --- UI에서 호출할 함수들 (BlueprintCallable) ---

	/** 1. (Instructions 단계) "준비 완료" 버튼 클릭 */
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void PlayerReady();

	/** 2. (Setup 단계) UI에서 정답 과일 5개 선택 완료 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	/** 3. 캐릭터(Pawn)의 IA_Grab에서 호출할 함수 */
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void RequestInteract(AActor* HitActor);

	/** (신규!) 4. UI 블루프린트에서 '돌림판 애니메이션 끝'을 알리고 턴 시작을 요청 */
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void RequestStartPlayerTurn();

	/** 로컬 정답 배열 Get 함수 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Fruit Game")
	const TArray<EFruitType>& GetMyLocalSecretAnswers() const;


	// --- 블루프린트 구현 이벤트 ---

	/** (신규!) 블루프린트에서 구현될 이벤트. 돌림판 애니메이션 재생 지시. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit Game|Animation")
	void PlaySpinnerAnimationEvent(int32 WinningPlayerIndex);


	// --- 서버 -> 클라이언트 RPC ---
	UFUNCTION(Client, Reliable)
	void Client_StartTurn();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveGuessResult(const TArray<EFruitType>& Guess, int32 MatchCount);

	UFUNCTION(Client, Reliable)
	void Client_OpponentGuessed(const TArray<EFruitType>& Guess, int32 MatchCount);

	UFUNCTION(Client, Reliable)
	void Client_GameOver(bool bYouWon);

	/** (신규!) 돌림판 애니메이션 재생 지시용 RPC */
	UFUNCTION(Client, Reliable)
	void Client_PlaySpinnerAnimation(int32 WinningPlayerIndex);


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
	// --- 클라이언트 -> 서버 RPC ---

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayerReady();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteract(AActor* HitActor);

	/** (신규!) RequestStartPlayerTurn이 호출할 서버 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartPlayerTurn();
};