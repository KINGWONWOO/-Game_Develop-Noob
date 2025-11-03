// FruitPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FruitGame/FruitGameTypes.h"
#include "FruitPlayerState.generated.h"

// --- 델리게이트 선언 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyStateChanged); // Setup 단계 완료
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInstructionReadyChanged); // Instructions 단계 준비 완료
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKnockdownStateChanged, bool, bIsKnockedDown); // (신규!) 쓰러짐 델리게이트

UCLASS()
class NOOBGAME_API AFruitPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFruitPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- 정답 배열 (보안) ---
private:
	TArray<EFruitType> SecretFruitAnswers;

public:
	// --- 서버 전용 함수 ---
	void SetSecretAnswers_Server(const TArray<EFruitType>& Answers);
	const TArray<EFruitType>& GetSecretAnswers_Server() const;
	void SetInstructionReady_Server();


	// --- 준비 상태 변수 (복제됨) ---
	UPROPERTY(ReplicatedUsing = OnRep_HasSubmittedFruits)
	bool bHasSubmittedFruits;

	UPROPERTY(ReplicatedUsing = OnRep_IsReady_Instructions)
	bool bIsReady_Instructions;

	// --- (신규!) 펀치 관련 변수 (복제됨) ---
	/** (신규!) 펀치 맞은 횟수 */
	UPROPERTY(Replicated)
	int32 PunchHitCount = 0;

	/** (신규!) 현재 쓰러져 있는지 여부 */
	UPROPERTY(ReplicatedUsing = OnRep_KnockedDown)
	bool bIsKnockedDown = false;

	/** (신규!) bIsKnockedDown 값이 클라이언트에서 변경될 때 호출될 함수 */
	UFUNCTION()
	void OnRep_KnockedDown();


	// --- UI 바인딩용 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "Player State")
	FOnReadyStateChanged OnReadyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Player State")
	FOnInstructionReadyChanged OnInstructionReadyChanged;

	/** (신규!) 캐릭터 BP가 이 델리게이트에 바인딩(구독)합니다. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnKnockdownStateChanged OnKnockdownStateChanged;

protected:
	UFUNCTION()
	void OnRep_HasSubmittedFruits();

	UFUNCTION()
	void OnRep_IsReady_Instructions();
};