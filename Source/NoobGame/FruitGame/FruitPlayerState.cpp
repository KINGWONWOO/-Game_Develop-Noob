
// FruitPlayerState.cpp

#include "FruitGame/FruitPlayerState.h"
#include "Net/UnrealNetwork.h"

AFruitPlayerState::AFruitPlayerState()
{
	bHasSubmittedFruits = false;
}

void AFruitPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// bHasSubmittedFruits만 복제합니다. SecretFruitAnswers는 절대 복제하지 않습니다.
	DOREPLIFETIME(AFruitPlayerState, bHasSubmittedFruits);
}

void AFruitPlayerState::SetSecretAnswers_Server(const TArray<EFruitType>& Answers)
{
	// 오직 서버(Authority)에서만 이 변수를 수정할 수 있도록 강제합니다.
	if (HasAuthority())
	{
		SecretFruitAnswers = Answers;
		bHasSubmittedFruits = true;

		// bHasSubmittedFruits가 true가 되었으므로, 모든 클라이언트에 복제될 것입니다.
		// 서버에서도 OnRep 함수를 수동으로 호출해주는 것이 좋습니다. (UI 갱신 등)
		OnRep_HasSubmittedFruits();
	}
}

const TArray<EFruitType>& AFruitPlayerState::GetSecretAnswers_Server() const
{
	// Get은 서버가 아니어도 호출은 가능하지만,
	// 클라이언트에서는 SecretFruitAnswers 배열이 비어있으므로(복제되지 않았으므로) 의미가 없습니다.
	return SecretFruitAnswers;
}

void AFruitPlayerState::OnRep_HasSubmittedFruits()
{
	// 준비 상태가 변경되었음을 UI(블루프린트)에 알립니다.
	OnReadyStateChanged.Broadcast();
}