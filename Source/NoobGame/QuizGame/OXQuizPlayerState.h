// OXQuizPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OXQuizPlayerState.generated.h"

UCLASS()
class NOOBGAME_API AOXQuizPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// ──────────────────────────────────────────────────────────────────────────
	// Constructor & Framework Overrides
	// ──────────────────────────────────────────────────────────────────────────
	AOXQuizPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ──────────────────────────────────────────────────────────────────────────
	// Public Server API (Setters called by GameMode/Controller)
	// ──────────────────────────────────────────────────────────────────────────

	// --- Ready Phase ---
	void SetInstructionReady_Server();

	// ──────────────────────────────────────────────────────────────────────────
	// Replicated Properties (Networked Variables)
	// ──────────────────────────────────────────────────────────────────────────
	UPROPERTY(Replicated)
	TSubclassOf<APawn> SelectedPawnClass;

	/** Instructions 단계 준비 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bIsReady_Instructions;

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