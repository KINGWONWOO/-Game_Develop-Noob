#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FruitGame/FruitGameTypes.h"
#include "Camera/CameraActor.h"
#include "FruitPlayerController.generated.h"

// --- 전방 선언 ---
class AInteractableFruitObject;
class ACharacter;
class UAnimMontage;
class ACameraActor; // ACameraActor 전방 선언 추가

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
	TArray<EFruitType> MyLocalSecretAnswers;

public:
	// --- UI 및 캐릭터에서 호출할 함수들 ---
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
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPunch(ACharacter* HitCharacter);
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPlayPunchMontage();

	/** [신규] 블루프린트에서 호출하여 이동 및 입력을 복구하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void RequestRestoreMovementAndInput();


	// --- 블루프린트 구현 이벤트 ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit Game|Animation")
	void PlaySpinnerAnimationEvent(int32 WinningPlayerIndex);
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayHitReactionOnCharacter(ACharacter* TargetCharacter);
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayPunchEvent(ACharacter* PunchingCharacter, bool bIsLeftPunch);

	/** 래그돌 카메라 효과 (비네팅) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Camera")
	void ApplyKnockdownCameraEffect(bool bEnableEffect);

	/** (수정!) 승자 타입과 나의 승리 여부를 블루프린트로 전달 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void Event_ShowResultsScreen(ECharacterType WinnerType, bool bYouWon);

	/** (신규!) 게임 종료 시 모든 입력/UI를 정리하고 연출 카메라로 전환하는 BP 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void Event_SetupResultsScreen();

	// --- 서버 -> 클라이언트 RPC ---
	UFUNCTION(Client, Reliable)
	void Client_StartTurn();
	UFUNCTION(Client, Reliable)
	void Client_ReceiveGuessResult(const TArray<EFruitType>& Guess, int32 MatchCount);
	UFUNCTION(Client, Reliable)
	void Client_OpponentGuessed(const TArray<EFruitType>& Guess, int32 MatchCount);
	UFUNCTION(Client, Reliable)
	void Client_PlaySpinnerAnimation(int32 WinningPlayerIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitReaction(ACharacter* TargetCharacter, UAnimMontage* MontageToPlay);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPunchMontage(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay);

	UFUNCTION(Client, Reliable)
	void Client_SetCameraEffect(bool bEnableKnockdownEffect);

	UFUNCTION(Client, Reliable)
	void Client_SetUIOnlyInput(bool bYouWon, ECharacterType WinnerType, ACameraActor* EndingCamera);

	/** [기존] 5초 후 게임 모드로 복구하는 RPC */
	UFUNCTION(Client, Reliable)
	void Client_SetGameOnlyInput();

	/** [수정] GameMode가 호출할 엔딩 처리 함수 (서버 실행, 카메라 참조 포함) */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetupEnding(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);
	bool Server_SetupEnding_Validate(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);
	void Server_SetupEnding_Implementation(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);


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
	bool Server_PlayerReady_Validate();
	void Server_PlayerReady_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);
	bool Server_SubmitSecretFruits_Validate(const TArray<EFruitType>& SecretFruits);
	void Server_SubmitSecretFruits_Implementation(const TArray<EFruitType>& SecretFruits);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteract(AActor* HitActor);
	bool Server_RequestInteract_Validate(AActor* HitActor);
	void Server_RequestInteract_Implementation(AActor* HitActor);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartPlayerTurn();
	bool Server_RequestStartPlayerTurn_Validate();
	void Server_RequestStartPlayerTurn_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPunch(ACharacter* HitCharacter);
	bool Server_RequestPunch_Validate(ACharacter* HitCharacter);
	void Server_RequestPunch_Implementation(ACharacter* HitCharacter);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPlayPunchMontage();
	bool Server_RequestPlayPunchMontage_Validate();
	void Server_RequestPlayPunchMontage_Implementation();

	/** [신규] RequestRestoreMovementAndInput이 호출할 서버 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestRestoreControls();
	bool Server_RequestRestoreControls_Validate();
	void Server_RequestRestoreControls_Implementation();
};