// FruitPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameTypes.h"
#include "GameFramework/Pawn.h"
#include "FruitPlayerState.generated.h"

UCLASS()
class NOOBGAME_API AFruitPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// ──────────────────────────────────────────────────────────────────────────
	// Constructor & Framework Overrides
	// ──────────────────────────────────────────────────────────────────────────
	AFruitPlayerState();

	/** 리플리케이션(복제) 설정 함수 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ──────────────────────────────────────────────────────────────────────────
	// Public Server API (Setters/Getters for GameMode)
	// ──────────────────────────────────────────────────────────────────────────
	void SetInstructionReady_Server();

	void SetSecretAnswers_Server(const TArray<EFruitType>& SecretFruits);

	const TArray<EFruitType>& GetSecretAnswers_Server() const;

	// ──────────────────────────────────────────────────────────────────────────
	// Replicated Properties (Configuration)
	// ──────────────────────────────────────────────────────────────────────────
	UPROPERTY(Replicated)
	TSubclassOf<APawn> SelectedPawnClass;

	// ──────────────────────────────────────────────────────────────────────────
	// Replicated Properties (Game Flow State)
	// ──────────────────────────────────────────────────────────────────────────

	/** Instructions 단계 준비 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bIsReady_Instructions;

	/** Setup 단계 과일 제출 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bHasSubmittedFruits;

	/** 이 플레이어의 비밀 정답 (서버에만 저장되고 복제됨) */
	UPROPERTY(Replicated)
	TArray<EFruitType> SecretAnswers;

	// ──────────────────────────────────────────────────────────────────────────
	// Replicated Properties (Combat State)
	// ──────────────────────────────────────────────────────────────────────────

	/** 현재 펀치 맞은 횟수 (쓰러짐 판정용) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	int32 PunchHitCount;

	/** 현재 쓰러진 상태인지 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	bool bIsKnockedDown;

	/** 다음 펀치가 왼쪽인지 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	bool bIsNextPunchLeft;
};