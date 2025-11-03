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
#include "GameFramework/Character.h" 
#include "Animation/AnimMontage.h"

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

// --- 4. SpinnerTurn -> PlayerTurn 단계 (UI) ---
void AFruitPlayerController::RequestStartPlayerTurn()
{
	Server_RequestStartPlayerTurn();
}
bool AFruitPlayerController::Server_RequestStartPlayerTurn_Validate() { return true; }
void AFruitPlayerController::Server_RequestStartPlayerTurn_Implementation()
{
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->PlayerRequestsStartTurn(this);
	}
}

// --- 5. 펀치 요청 (적중) ---
void AFruitPlayerController::RequestPunch(ACharacter* HitCharacter)
{
	Server_RequestPunch(HitCharacter); // 서버 RPC 호출
}
bool AFruitPlayerController::Server_RequestPunch_Validate(ACharacter* HitCharacter) { return HitCharacter != nullptr; }
void AFruitPlayerController::Server_RequestPunch_Implementation(ACharacter* HitCharacter)
{
	// GameMode에게 실제 처리 위임
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();
	if (GM)
	{
		GM->ProcessPunch(this, HitCharacter);
	}
}

// --- 6. 펀치 요청 (애니메이션) ---
void AFruitPlayerController::RequestPlayPunchMontage()
{
	Server_RequestPlayPunchMontage();
}
bool AFruitPlayerController::Server_RequestPlayPunchMontage_Validate()
{
	return true;
}
/** (수정!) 서버 RPC가 Multicast를 직접 호출하는 대신 GameMode에 위임합니다. */
void AFruitPlayerController::Server_RequestPlayPunchMontage_Implementation()
{
	ACharacter* MyCharacter = GetPawn<ACharacter>();
	AFruitGameMode* GM = GetWorld()->GetAuthGameMode<AFruitGameMode>();

	if (MyCharacter && GM)
	{
		// GameMode에게 "이 캐릭터"의 펀치 애니메이션을 모두에게 전파하라고 요청
		GM->ProcessPunchAnimation(MyCharacter);
	}
}


// --- 서버 -> 클라이언트 RPC 구현 ---
void AFruitPlayerController::Client_StartTurn_Implementation() { OnTurnStarted.Broadcast(); }
void AFruitPlayerController::Client_ReceiveGuessResult_Implementation(const TArray<EFruitType>& Guess, int32 MatchCount) { OnGuessResultReceived.Broadcast(Guess, MatchCount); }
void AFruitPlayerController::Client_OpponentGuessed_Implementation(const TArray<EFruitType>& Guess, int32 MatchCount) { OnOpponentGuessReceived.Broadcast(Guess, MatchCount); }
void AFruitPlayerController::Client_GameOver_Implementation(bool bYouWon) { OnGameOver.Broadcast(bYouWon); }
void AFruitPlayerController::Client_PlaySpinnerAnimation_Implementation(int32 WinningPlayerIndex)
{
	PlaySpinnerAnimationEvent(WinningPlayerIndex);
}

/** 피격 애니메이션 Multicast RPC 구현부 */
void AFruitPlayerController::Multicast_PlayHitReaction_Implementation(ACharacter* TargetCharacter)
{
	PlayHitReactionOnCharacter(TargetCharacter);
}

/** 펀치 애니메이션 Multicast RPC 구현부 */
void AFruitPlayerController::Multicast_PlayPunchMontage_Implementation(ACharacter* PunchingCharacter)
{
	// 몽타주 애셋이 유효하고, 캐릭터가 유효하면
	if (PunchingCharacter && PunchMontageAsset)
	{
		// 캐릭터의 스켈레탈 메시에서 몽타주를 재생합니다.
		PunchingCharacter->PlayAnimMontage(PunchMontageAsset);
	}
}