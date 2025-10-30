// FruitPlayerController.cpp

#include "FruitGame/FruitPlayerController.h"
#include "FruitGame/FruitGameMode.h"
#include "FruitGame/FruitPlayerState.h"
#include "FruitGame/FruitGameState.h"
#include "FruitGame/InteractableFruitObject.h"
#include "FruitGame/SubmitGuessButton.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h" 

// --- Get 함수 ---
const TArray<EFruitType>& AFruitPlayerController::GetMyLocalSecretAnswers() const
{
	return MyLocalSecretAnswers;
}

// --- 캐릭터(Pawn) -> 컨트롤러 ---
void AFruitPlayerController::RequestInteract(AActor* HitActor)
{
	Server_RequestInteract(HitActor);
}

// --- 1. Instructions 단계 (UI) ---
void AFruitPlayerController::PlayerReady()
{
	Server_PlayerReady();
}
bool AFruitPlayerController::Server_PlayerReady_Validate() { return true; }
void AFruitPlayerController::Server_PlayerReady_Implementation()
{
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->PlayerIsReady(this);
	}
}

// --- 2. Setup 단계 (UI) ---
void AFruitPlayerController::SubmitSecretFruits(const TArray<EFruitType>& SecretFruits)
{
	if (IsLocalController())
	{
		MyLocalSecretAnswers = SecretFruits;
	}
	Server_SubmitSecretFruits(SecretFruits);
}
bool AFruitPlayerController::Server_SubmitSecretFruits_Validate(const TArray<EFruitType>& SecretFruits) { return SecretFruits.Num() == 5; }
void AFruitPlayerController::Server_SubmitSecretFruits_Implementation(const TArray<EFruitType>& SecretFruits)
{
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->PlayerSubmittedFruits(this, SecretFruits);
	}
}

// --- 3. PlayerTurn 단계 (월드 상호작용) ---
bool AFruitPlayerController::Server_RequestInteract_Validate(AActor* HitActor) { return HitActor != nullptr; }
void AFruitPlayerController::Server_RequestInteract_Implementation(AActor* HitActor)
{
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	AFruitGameState* GS = GetWorld()->GetGameState<AFruitGameState>();
	if (GM && GS && HitActor)
	{
		GM->PlayerInteracted(this, HitActor, GS->CurrentGamePhase);
	}
}

// --- (신규!) 4. SpinnerTurn -> PlayerTurn 단계 (UI) ---
void AFruitPlayerController::RequestStartPlayerTurn()
{
	// 서버에 턴 시작을 요청
	Server_RequestStartPlayerTurn();
}
bool AFruitPlayerController::Server_RequestStartPlayerTurn_Validate()
{
	return true;
}
void AFruitPlayerController::Server_RequestStartPlayerTurn_Implementation()
{
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->PlayerRequestsStartTurn(this);
	}
}


// --- 서버 -> 클라이언트 RPC 구현 ---
void AFruitPlayerController::Client_StartTurn_Implementation() { OnTurnStarted.Broadcast(); }
void AFruitPlayerController::Client_ReceiveGuessResult_Implementation(const TArray<EFruitType>& Guess, int32 MatchCount) { OnGuessResultReceived.Broadcast(Guess, MatchCount); }
void AFruitPlayerController::Client_OpponentGuessed_Implementation(const TArray<EFruitType>& Guess, int32 MatchCount) { OnOpponentGuessReceived.Broadcast(Guess, MatchCount); }
void AFruitPlayerController::Client_GameOver_Implementation(bool bYouWon) { OnGameOver.Broadcast(bYouWon); }

/** (신규!) 돌림판 애니메이션 RPC 구현부 */
void AFruitPlayerController::Client_PlaySpinnerAnimation_Implementation(int32 WinningPlayerIndex)
{
	// 블루프린트 이벤트 호출
	PlaySpinnerAnimationEvent(WinningPlayerIndex);
}