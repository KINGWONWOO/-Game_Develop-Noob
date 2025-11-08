// NoobGameCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NoobGameCharacter.generated.h"

class UAnimMontage;

UCLASS()
class NOOBGAME_API ANoobGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANoobGameCharacter();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

	// (수정!) 입력 바인딩 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 래그돌 상태를 실제 켜고 끄는 함수 */
	void SetRagdoll(bool bEnable);

	/** 서버(GameMode)에서 래그돌 상태를 변경하기 위한 public 함수 */
	void SetRagdollState_Server(bool bEnable);

	/** (신규!) 게임 종료 애니메이션을 모든 클라이언트에 재생시킵니다 (서버가 호출) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEndGameAnim(bool bIsWinner);

	// --- 몽타주 애셋 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Montages")
	UAnimMontage* LeftPunchMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* RightPunchMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit Reaction")
	UAnimMontage* HitReaction_Front;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit Reaction")
	UAnimMontage* HitReaction_Back;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit Reaction")
	UAnimMontage* HitReaction_Left;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit Reaction")
	UAnimMontage* HitReaction_Right;

	/** (신규!) 승리 포즈 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Montages")
	UAnimMontage* VictoryMontage;

	/** (신규!) 패배 포즈 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Montages")
	UAnimMontage* DefeatMontage;

	// --- (신규!) 카메라 제어 변수 ---

	/** (신규!) 마우스 감도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraSensitivity;

	/** (신규!) 마우스 X축 반전 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool ReverseX;

	/** (신규!) 마우스 Y축 반전 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool ReverseY;


protected:
	/** 래그돌 상태 (서버에서 변경되면 클라이언트로 복제됨) */
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdolling)
	bool bIsRagdolling;

	/** bIsRagdolling 변수가 복제될 때 클라이언트에서 호출될 함수 */
	UFUNCTION()
	void OnRep_IsRagdolling();

	/** 리플리케이션(복제) 설정 함수 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 회복 애니메이션이 끝난 후 이동을 활성화하기 위한 타이머 */
	FTimerHandle RecoveryMovementTimerHandle;

	/** 타이머가 만료되면 이동을 활성화하는 함수 */
	void EnableMovementAfterRecovery();

	/** 카메라 시점을 부드럽게 복구하기 위한 타이머 */
	FTimerHandle CameraRecoveryTimer;

	/** 카메라가 복구되어야 할 목표 회전값 (Yaw) */
	FRotator TargetControlRotation;

	/** 카메라 보간 타이머가 매 틱 호출할 함수 */
	UFUNCTION()
	void UpdateCameraRecovery();

	// --- (신규!) 카메라 입력 처리 함수 ---

	/** (신규!) 마우스 좌우 (Turn) 입력을 처리합니다. */
	void Turn(float Value);

	/** (신규!) 마우스 상하 (LookUp) 입력을 처리합니다. */
	void LookUp(float Value);

	// --- 물리 블렌딩 변수 ---

	/** 래그돌에서 애니메이션으로 블렌딩 중인지 여부 */
	bool bIsBlendingFromRagdoll;

	/** 현재 물리 블렌드 가중치 (1.0 = 래그돌, 0.0 = 애니메이션) */
	float RagdollBlendWeight;

	/** 블렌딩에 걸리는 시간 (초) */
	float RagdollBlendDuration;
};