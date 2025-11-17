#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameTypes.h"
#include "Camera/CameraActor.h"
#include "OXQuizPlayerController.generated.h"

// --- Forward Declarations ---
class UAnimMontage;
class ACameraActor;
class UUserWidget;
class AOXQuizPlayerState;
class ACharacter;

// --- Delegates ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOXGameOver, bool, bYouWon);

UCLASS()
class NOOBGAME_API AOXQuizPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// ──────────────────────────────────────────────────────────────────────────
	// Public Interface (UI & Input) - BlueprintCallable
	// ──────────────────────────────────────────────────────────────────────────

	// -- Quiz Flow --
	UFUNCTION(BlueprintCallable, Category = "Quiz")
	void PlayerReady();

	// -- Combat --
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPunch(ACharacter* HitCharacter);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestPlayPunchMontage();

	// ──────────────────────────────────────────────────────────────────────────
	// Blueprint Implementable Events (To be implemented in BP)
	// ──────────────────────────────────────────────────────────────────────────

	// -- Animation / Visuals --
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayHitReactionOnCharacter(ACharacter* TargetCharacter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayPunchEvent(ACharacter* PunchingCharacter, bool bIsLeftPunch);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quiz|Animation")
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
	void Server_RequestPunch(ACharacter* HitCharacter);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestPlayPunchMontage();

	/** [Server] GameMode가 호출하는 엔딩 처리 함수 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetupEnding(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);
	// ──────────────────────────────────────────────────────────────────────────
	// Client RPCs (Server -> Client)
	// ──────────────────────────────────────────────────────────────────────────

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
	// Delegates & Variables
	// ──────────────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable)
	FOnOXGameOver OnGameOver;

protected:
	// ──────────────────────────────────────────────────────────────────────────
	// Protected Internal Helpers & Overrides
	// ──────────────────────────────────────────────────────────────────────────

	UFUNCTION()
	void OnEndingMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// -- UI System --
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainUIClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> MainUIInstance;
};