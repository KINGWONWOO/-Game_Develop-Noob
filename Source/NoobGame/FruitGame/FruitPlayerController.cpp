// FruitPlayerController.cpp

#include "FruitGame/FruitPlayerController.h"
#include "FruitGameMode.h" // GameMode를 호출하기 위해 포함
#include "Engine/World.h"

// --- 1. UI -> C++ (클라이언트) ---

void AFruitPlayerController::SubmitSecretFruits(const TArray<EFruitType>& SecretFruits)
{
	// 서버로 RPC 호출
	Server_SubmitSecretFruits(SecretFruits);
}

void AFruitPlayerController::SubmitGuess(const TArray<EFruitType>& GuessedFruits)
{
	// 서버로 RPC 호출
	Server_SubmitGuess(GuessedFruits);
}

// --- 2. C++ (클라이언트) -> C++ (서버) ---

bool AFruitPlayerController::Server_SubmitSecretFruits_Validate(const TArray<EFruitType>& SecretFruits)
{
	return SecretFruits.Num() == 5; // 간단한 유효성 검사 (5개가 아니면 RPC 거부)
}

void AFruitPlayerController::Server_SubmitSecretFruits_Implementation(const TArray<EFruitType>& SecretFruits)
{
	// 서버에서 실행되는 코드
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->PlayerSubmittedFruits(this, SecretFruits);
	}
}

bool AFruitPlayerController::Server_SubmitGuess_Validate(const TArray<EFruitType>& GuessedFruits)
{
	return GuessedFruits.Num() == 5; // 간단한 유효성 검사
}

void AFruitPlayerController::Server_SubmitGuess_Implementation(const TArray<EFruitType>& GuessedFruits)
{
	// 서버에서 실행되는 코드
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->ProcessPlayerGuess(this, GuessedFruits);
	}
}

// --- 3. C++ (서버) -> C++ (클라이언트) ---

void AFruitPlayerController::Client_StartTurn_Implementation()
{
	// 클라이언트에서 실행되는 코드
	// UI 위젯에 "내 턴"임을 알림
	OnTurnStarted.Broadcast();
}

void AFruitPlayerController::Client_ReceiveGuessResult_Implementation(const TArray<EFruitType>& Guess, int32 MatchCount)
{
	// 클라이언트에서 실행되는 코드
	// UI 위젯에 "N개 맞춤" 표시
	OnGuessResultReceived.Broadcast(Guess, MatchCount);
}

void AFruitPlayerController::Client_OpponentGuessed_Implementation(const TArray<EFruitType>& Guess, int32 MatchCount)
{
	// 클라이언트에서 실행되는 코드
	// UI 위젯에 "상대방이 추측: N개 맞춤" 표시
	OnOpponentGuessReceived.Broadcast(Guess, MatchCount);
}

void AFruitPlayerController::Client_GameOver_Implementation(bool bYouWon)
{
	// 클라이언트에서 실행되는 코드
	// UI 위젯에 "승리/패배" 표시
	OnGameOver.Broadcast(bYouWon);
}