#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FruitGame/FruitGameTypes.h"
#include "Camera/CameraActor.h"
#include "FruitPlayerController.generated.h"

// --- ���� ���� ---
class AInteractableFruitObject;
class ACharacter;
class UAnimMontage;
class ACameraActor; // ACameraActor ���� ���� �߰�

// --- ��������Ʈ ���� ---
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
	// --- UI �� ĳ���Ϳ��� ȣ���� �Լ��� ---
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

	UFUNCTION(Client, Reliable)
	void Client_EnableMovementAfterEnding();

	// --- ��������Ʈ ���� �̺�Ʈ ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit Game|Animation")
	void PlaySpinnerAnimationEvent(int32 WinningPlayerIndex);
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayHitReactionOnCharacter(ACharacter* TargetCharacter);
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayPunchEvent(ACharacter* PunchingCharacter, bool bIsLeftPunch);

	/** ���׵� ī�޶� ȿ�� (�����) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Camera")
	void ApplyKnockdownCameraEffect(bool bEnableEffect);

	/** (����!) ���� Ÿ�԰� ���� �¸� ���θ� ��������Ʈ�� ���� */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void Event_ShowResultsScreen(ECharacterType WinnerType, bool bYouWon);

	/** (�ű�!) ���� ���� �� ��� �Է�/UI�� �����ϰ� ���� ī�޶�� ��ȯ�ϴ� BP �̺�Ʈ */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void Event_SetupResultsScreen();

	// --- ���� -> Ŭ���̾�Ʈ RPC ---
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

	/** [����] GameMode�� ȣ���� ���� ó�� �Լ� (���� ����, ī�޶� ���� ����) */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetupEnding(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);
	bool Server_SetupEnding_Validate(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);
	void Server_SetupEnding_Implementation(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera);


	// --- UI ���ε��� ��������Ʈ ---
	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnTurnStarted OnTurnStarted;
	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnGuessResultReceived OnGuessResultReceived;
	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnOpponentGuessReceived OnOpponentGuessReceived;
	UPROPERTY(BlueprintAssignable, Category = "Fruit Game")
	FOnGameOver OnGameOver;

protected:

	UFUNCTION()
	void OnEndingMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// --- Ŭ���̾�Ʈ -> ���� RPC ---
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
};