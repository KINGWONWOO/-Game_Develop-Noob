#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameTypes.h"
#include "Camera/CameraActor.h"
#include "FruitPlayerController.generated.h"

// --- Forward Declarations ---
class AInteractableFruitObject;
class ACharacter;
class UAnimMontage;
class ACameraActor;

// --- Delegates ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuessResultReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOpponentGuessReceived, const TArray<EFruitType>&, Guess, int32, MatchCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bYouWon);

UCLASS()
class NOOBGAME_API AFruitPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// ──────────────────────────────────────────────────────────────────────────
	// Public Interface (UI & Input) - BlueprintCallable
	// ──────────────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void PlayerReady();

	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void RequestInteract(AActor* HitActor);

	UFUNCTION(BlueprintCallable, Category = "Fruit Game")
	void RequestStartPlayerTurn();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPunch(ACharacter* HitCharacter);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPlayPunchMontage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Fruit Game")
	const TArray<EFruitType>& GetMyLocalSecretAnswers() const;


	// ──────────────────────────────────────────────────────────────────────────
	// Blueprint Implementable Events (To be implemented in BP)
	// ──────────────────────────────────────────────────────────────────────────

	// -- Animation / Visuals --
	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit Game|Animation")
	void PlaySpinnerAnimationEvent(int32 WinningPlayerIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayHitReactionOnCharacter(ACharacter* TargetCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayPunchEvent(ACharacter* PunchingCharacter, bool bIsLeftPunch);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Camera")
	void ApplyKnockdownCameraEffect(bool bEnableEffect);

	// -- UI / Game Flow --
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void Event_ShowResultsScreen(ECharacterType WinnerType, bool bYouWon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void Event_SetupResultsScreen();


	// ──────────────────────────────────────────────────────────────────────────
	// Server RPCs (Client -> Server)
	// ──────────────────────────────────────────────────────────────────────────
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayerReady();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SubmitSecretFruits(const TArray<EFruitType>& SecretFruits);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteract(AActor* HitActor);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartPlayerTurn();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPunch(ACharacter* HitCharacter);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPlayPunchMontage();

	/** [Server] GameMode가 호출하는 엔딩 처리 함수 (폰 이동, 카메라 설정 등) */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetupEnding(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);


	// ──────────────────────────────────────────────────────────────────────────
	// Client RPCs (Server -> Client)
	// ──────────────────────────────────────────────────────────────────────────
	UFUNCTION(Client, Reliable)
	void Client_StartTurn();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveGuessResult(const TArray<EFruitType>& Guess, int32 MatchCount);

	UFUNCTION(Client, Reliable)
	void Client_OpponentGuessed(const TArray<EFruitType>& Guess, int32 MatchCount);

	UFUNCTION(Client, Reliable)
	void Client_PlaySpinnerAnimation(int32 WinningPlayerIndex);

	UFUNCTION(Client, Reliable)
	void Client_SetCameraEffect(bool bEnableKnockdownEffect);

	UFUNCTION(Client, Reliable)
	void Client_SetUIOnlyInput(bool bYouWon, ECharacterType WinnerType, ACameraActor* EndingCamera);

	UFUNCTION(Client, Reliable)
	void Client_EnableMovementAfterEnding();


	// ──────────────────────────────────────────────────────────────────────────
	// NetMulticast RPCs (Server -> All Clients)
	// ──────────────────────────────────────────────────────────────────────────
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitReaction(ACharacter* TargetCharacter, UAnimMontage* MontageToPlay);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPunchMontage(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay);


	// ──────────────────────────────────────────────────────────────────────────
	// Delegates (UI Binding)
	// ──────────────────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnTurnStarted OnTurnStarted;

	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnGuessResultReceived OnGuessResultReceived;

	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnOpponentGuessReceived OnOpponentGuessReceived;

	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnGameOver OnGameOver;


protected:
	// ──────────────────────────────────────────────────────────────────────────
	// Internal Helpers & Callbacks
	// ──────────────────────────────────────────────────────────────────────────
	UFUNCTION()
	void OnEndingMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	TArray<EFruitType> MyLocalSecretAnswers;
};