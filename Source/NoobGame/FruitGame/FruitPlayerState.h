// FruitPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FruitGame/FruitGameTypes.h"
#include "GameFramework/Pawn.h"
#include "FruitPlayerState.generated.h"

UCLASS()
class NOOBGAME_API AFruitPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFruitPlayerState();

	UPROPERTY(Replicated)
	TSubclassOf<APawn> SelectedPawnClass;

	// --- (신규!) 블루프린트(UI) 및 서버(GameMode)에서 읽어야 하는 변수들 ---

	/** (수정!) Instructions 단계 준비 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bIsReady_Instructions;

	/** (수정!) Setup 단계 과일 제출 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bHasSubmittedFruits;

	/** (수정!) 이 플레이어의 비밀 정답 (서버에만 저장되고 복제됨) */
	UPROPERTY(Replicated)
	TArray<EFruitType> SecretAnswers;

	/** (수정!) 현재 펀치 맞은 횟수 (쓰러짐 판정용) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	int32 PunchHitCount;

	/** (수정!) 현재 쓰러진 상태인지 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	bool bIsKnockedDown;

	/** (수정!) 다음 펀치가 왼쪽인지 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	bool bIsNextPunchLeft;

	// --- (기존 함수들) ---
	// (이 함수들은 서버에서 호출되어야 합니다)
	void SetInstructionReady_Server();
	void SetSecretAnswers_Server(const TArray<EFruitType>& SecretFruits);
	const TArray<EFruitType>& GetSecretAnswers_Server() const;

protected:
	/** (신규!) 리플리케이션(복제) 설정 함수 선언 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};