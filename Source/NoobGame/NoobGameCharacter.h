#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NoobGameCharacter.generated.h"

class UAnimMontage;
class USoundBase;
class USoundAttenuation;

UCLASS()
class NOOBGAME_API ANoobGameCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANoobGameCharacter();

    // --- 프레임워크 기본 인터페이스 ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 시선 입력 처리
    UFUNCTION(BlueprintCallable, Category = "Input")
    void Turn(float Value);
    UFUNCTION(BlueprintCallable, Category = "Input")
    void LookUp(float Value);

    // --- 카메라 및 설정 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraSensitivity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool ReverseX;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool ReverseY;


    // --- 전투 및 피해 처리 ---
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(Server, Reliable)
    void Server_PlayHitSound();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayHitSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayEndGameAnim(bool bIsWinner);

    // --- 상태 및 애니메이션 ---
    void SetDownState_Server(bool bInDown);
    FORCEINLINE bool GetIsDown() const { return bIsDown; }

    // 공격 및 반응 몽타주 애셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    UAnimMontage* LeftPunchMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    UAnimMontage* RightPunchMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit")
    UAnimMontage* HitReaction_Front;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit")
    UAnimMontage* HitReaction_Back;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit")
    UAnimMontage* HitReaction_Left;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Hit")
    UAnimMontage* HitReaction_Right;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|State")
    UAnimMontage* KnockdownMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|State")
    UAnimMontage* GetUpMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Result")
    UAnimMontage* VictoryMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Result")
    UAnimMontage* DefeatMontage;

    // --- 오디오 애셋 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* HitSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundAttenuation* HitAttenuation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* KnockdownSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundAttenuation* KnockdownAttenuation;

protected:
    // 기절 상태 복제 변수
    UPROPERTY(ReplicatedUsing = OnRep_IsDown)
    bool bIsDown;

    UFUNCTION()
    void OnRep_IsDown();

    void EnableMovementAfterRecovery();

    // 카메라 높이 조절 시스템 (최적화 타이머 기반)
    FTimerHandle CameraInterpTimerHandle;
    float TargetCameraZ;
    void UpdateCameraHeight();

    FTimerHandle RecoveryTimerHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FirstPersonCameraComponent;

    // 공격 종료 시 실행될 함수 (서버 권한)
    void ResetAttackState();
};
