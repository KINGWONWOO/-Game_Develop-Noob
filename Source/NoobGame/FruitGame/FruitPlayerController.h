// FruitPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FruitGame/FruitGameTypes.h"
#include "FruitPlayerController.generated.h"

// --- 전방 선언 ---
class AInteractableFruitObject;
class ACharacter;
class UAnimMontage; // 펀치 몽타주 애셋을 위해 추가

// --- 델리게이트 선언 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuessResultReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOpponentGuessReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bYouWon);


UCLASS()
class NOOBGAME_API AFruitPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	// 로컬 정답 저장용 변수
	TArray<EFruitType> MyLocalSecretAnswers;

public:
	// --- UI 및 캐릭터에서 호출할 함수들 (BlueprintCallable) ---
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void PlayerReady();

	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void RequestInteract(AActor* HitActor);

	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void RequestStartPlayerTurn();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Fruit Game")
	const TArray<EFruitType>& GetMyLocalSecretAnswers() const;

	/** 펀치 '적중' 요청 함수 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPunch(ACharacter* HitCharacter);

	/** 펀치 '애니메이션' 재생 요청 함수 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPlayPunchMontage();


	// --- 블루프린트 구현 이벤트 ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit Game|Animation")
	void PlaySpinnerAnimationEvent(int32 WinningPlayerIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayHitReactionOnCharacter(ACharacter* TargetCharacter);


	// --- 서버 -> 클라이언트 RPC ---
	UFUNCTION(Client, Reliable)
	void Client_StartTurn();
	UFUNCTION(Client, Reliable)
	void Client_ReceiveGuessResult(const TArray<EFruitType>& Guess, int32 MatchCount);
	UFUNCTION(Client, Reliable)
	void Client_OpponentGuessed(const TArray<EFruitType>& Guess, int32 MatchCount);
	UFUNCTION(Client, Reliable)
	void Client_GameOver(bool bYouWon);
	UFUNCTION(Client, Reliable)
	void Client_PlaySpinnerAnimation(int32 WinningPlayerIndex);

	/** (수정!) 모든 클라이언트에게 펀치 애니메이션 재생을 지시 (public으로 이동) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPunchMontage(ACharacter* PunchingCharacter);

	/** (수정!) 피격 애니메이션 RPC (public으로 이동) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitReaction(ACharacter* TargetCharacter);


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
	/** 펀치 몽타주 애셋 (BP_FruitPlayerController에서 할당) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* PunchMontageAsset;

	// --- 클라이언트 -> 서버 RPC ---
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayerReady();
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteract(AActor* HitActor);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartPlayerTurn();

	/** 펀치 '적중' 서버 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPunch(ACharacter* HitCharacter);

	/** 펀치 '애니메이션' 서버 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPlayPunchMontage();
};