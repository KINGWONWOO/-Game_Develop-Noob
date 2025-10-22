// FruitPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FruitGameTypes.h"
#include "FruitPlayerState.generated.h"

// UI 바인딩을 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyStateChanged);

UCLASS()
class NOOBGAME_API AFruitPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFruitPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 정답 과일 배열 (UPROPERTY 없음! 복제(Replicate)되지 않음!)
	// 이 변수는 서버에만 존재합니다.
private:
	TArray<EFruitType> SecretFruitAnswers;

public:
	// 서버에서만 이 함수를 호출하여 정답을 설정합니다.
	void SetSecretAnswers_Server(const TArray<EFruitType>& Answers);

	// 서버에서만 이 함수를 호출하여 정답을 가져옵니다.
	const TArray<EFruitType>& GetSecretAnswers_Server() const;

	// 플레이어가 정답 제출을 완료했는지 여부 (상대방 UI에 표시용)
	UPROPERTY(ReplicatedUsing = OnRep_HasSubmittedFruits)
	bool bHasSubmittedFruits;

	// UI 알림용 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Player State")
	FOnReadyStateChanged OnReadyStateChanged;

protected:
	UFUNCTION()
	void OnRep_HasSubmittedFruits();
};