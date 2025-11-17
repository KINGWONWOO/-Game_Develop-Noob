#include "FruitGame/FruitPlayerState.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME을 위해 필요

AFruitPlayerState::AFruitPlayerState()
{
	// 변수 초기화
	bIsReady_Instructions = false;
	bHasSubmittedFruits = false;
	PunchHitCount = 0;
	bIsKnockedDown = false;
	bIsNextPunchLeft = true;
}

/** 리플리케이션(복제)할 변수 등록 */
void AFruitPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFruitPlayerState, bIsReady_Instructions);
	DOREPLIFETIME(AFruitPlayerState, bHasSubmittedFruits);
	DOREPLIFETIME(AFruitPlayerState, SecretAnswers);
	DOREPLIFETIME(AFruitPlayerState, PunchHitCount);
	DOREPLIFETIME(AFruitPlayerState, bIsKnockedDown);
	DOREPLIFETIME(AFruitPlayerState, bIsNextPunchLeft);
	DOREPLIFETIME(AFruitPlayerState, SelectedPawnClass);
}

// 이 함수들은 PlayerController의 Server RPC에서 호출되어야 함.
void AFruitPlayerState::SetInstructionReady_Server()
{
	if (HasAuthority())
	{
		bIsReady_Instructions = true;
		// OnRep은 클라이언트에서 자동 호출
	}
}

void AFruitPlayerState::SetSecretAnswers_Server(const TArray<EFruitType>& SecretFruits)
{
	// 이 코드는 서버에서만 실행되어야 함.
	SecretAnswers = SecretFruits;
	bHasSubmittedFruits = true;
}

const TArray<EFruitType>& AFruitPlayerState::GetSecretAnswers_Server() const
{
	return SecretAnswers;
}