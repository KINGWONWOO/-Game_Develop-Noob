// FruitPlayerState.cpp

#include "FruitGame/FruitPlayerState.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME

AFruitPlayerState::AFruitPlayerState()
{
	bHasSubmittedFruits = false;
	bIsReady_Instructions = false;
	PunchHitCount = 0;
	bIsKnockedDown = false;
}

void AFruitPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFruitPlayerState, bHasSubmittedFruits);
	DOREPLIFETIME(AFruitPlayerState, bIsReady_Instructions);
	DOREPLIFETIME(AFruitPlayerState, PunchHitCount); // (신규) 복제 등록
	DOREPLIFETIME(AFruitPlayerState, bIsKnockedDown); // (신규) 복제 등록
}

// --- 서버 전용 함수 (변경 없음) ---
void AFruitPlayerState::SetSecretAnswers_Server(const TArray<EFruitType>& Answers)
{
	if (HasAuthority())
	{
		SecretFruitAnswers = Answers;
		bHasSubmittedFruits = true;
		OnRep_HasSubmittedFruits();
	}
}
const TArray<EFruitType>& AFruitPlayerState::GetSecretAnswers_Server() const
{
	return SecretFruitAnswers;
}
void AFruitPlayerState::SetInstructionReady_Server()
{
	if (HasAuthority())
	{
		if (!bIsReady_Instructions)
		{
			bIsReady_Instructions = true;
			OnRep_IsReady_Instructions();
		}
	}
}

// --- OnRep 함수 (클라이언트에서 실행됨) ---
void AFruitPlayerState::OnRep_HasSubmittedFruits()
{
	OnReadyStateChanged.Broadcast();
}
void AFruitPlayerState::OnRep_IsReady_Instructions()
{
	OnInstructionReadyChanged.Broadcast();
}

/** (신규!) 쓰러짐 상태가 복제될 때 델리게이트 호출 */
void AFruitPlayerState::OnRep_KnockedDown()
{
	OnKnockdownStateChanged.Broadcast(bIsKnockedDown);
}