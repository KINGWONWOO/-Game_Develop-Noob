#include "NoobGame/NoobGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// =============================================================
// 1. 초기화 및 설정 (Initialization)
// =============================================================

ANoobGameCharacter::ANoobGameCharacter()
{
    PrimaryActorTick.bCanEverTick = true; // Tick은 카메라 로직 제외를 위해 유지

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    // 메쉬 기본 위치 설정
    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -97.0f), FRotator(0.0f, -90.0f, 0.0f));

    // 네트워크 설정
    bReplicates = true;

    CameraSensitivity = 1.0f;
    ReverseX = false;
    ReverseY = false;
    bIsDown = false;
    TargetCameraZ = 130.0f;
}

void ANoobGameCharacter::BeginPlay()
{
    Super::BeginPlay();
    // 런타임에 카메라 컴포넌트 캐싱
    FirstPersonCameraComponent = Cast<UCameraComponent>(GetComponentByClass(UCameraComponent::StaticClass()));
}

void ANoobGameCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANoobGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANoobGameCharacter, bIsDown);
}

// =============================================================
// 2. 입력 처리 (Input & Movement)
// =============================================================

void ANoobGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("Turn", this, &ANoobGameCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ANoobGameCharacter::LookUp);
}

void ANoobGameCharacter::Turn(float Value)
{
    if (Value != 0.0f && Controller != nullptr && !bIsDown)
    {
        const float FinalValue = Value * CameraSensitivity * (ReverseX ? -1.0f : 1.0f);
		UE_LOG(LogTemp, Log, TEXT("Turn Input Value: %f, Final Value: %f"), Value, FinalValue);
        AddControllerYawInput(FinalValue);
    }
}

void ANoobGameCharacter::LookUp(float Value)
{
    if (Value != 0.0f && Controller != nullptr && !bIsDown)
    {
        const float FinalValue = Value * CameraSensitivity * (ReverseY ? -1.0f : 1.0f);
        AddControllerPitchInput(FinalValue);
    }
}

void ANoobCharacter::ResetAttackState()
{
    if (!HasAuthority()) return;

    // 타이머 초기화 및 상태 해제
    GetWorld()->GetTimerManager().ClearTimer(AttackSafeTimerHandle);
    bIsAttacking = false;
    
    // 로그를 남겨 Fail-Safe 작동 여부 확인 가능
    UE_LOG(LogTemp, Log, TEXT("Attack State Reset (Fail-Safe or Notify)"));
}

// =============================================================
// 3. 기절 및 회복 시스템 (State & Animation)
// =============================================================

void ANoobGameCharacter::SetDownState_Server(bool bInDown)
{
    if (HasAuthority())
    {
        if (bIsDown == bInDown) return;
        bIsDown = bInDown;

        // 서버 측 카메라 보간 시작
        TargetCameraZ = bIsDown ? -40.0f : 60.0f;
        GetWorldTimerManager().ClearTimer(CameraInterpTimerHandle);
        GetWorldTimerManager().SetTimer(CameraInterpTimerHandle, this, &ANoobGameCharacter::UpdateCameraHeight, 0.01f, true);

        OnRep_IsDown();
    }
}

void ANoobGameCharacter::OnRep_IsDown()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    // 클라이언트 측 카메라 동기화 보간
    TargetCameraZ = bIsDown ? 30.0f : 130.0f;
    GetWorldTimerManager().ClearTimer(CameraInterpTimerHandle);
    GetWorldTimerManager().SetTimer(CameraInterpTimerHandle, this, &ANoobGameCharacter::UpdateCameraHeight, 0.01f, true);

    if (bIsDown)
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
        if (KnockdownMontage) AnimInstance->Montage_Play(KnockdownMontage);

        if (KnockdownSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, KnockdownSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, KnockdownAttenuation);
        }
    }
    else
    {
        if (GetUpMontage)
        {
            AnimInstance->Montage_Play(GetUpMontage);
            float Duration = GetUpMontage->GetPlayLength();
            GetWorldTimerManager().SetTimer(RecoveryTimerHandle, this, &ANoobGameCharacter::EnableMovementAfterRecovery, Duration, false);
        }
    }
}

void ANoobGameCharacter::EnableMovementAfterRecovery()
{
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

// =============================================================
// 4. 카메라 최적화 보간 (Camera Interpolation)
// =============================================================

void ANoobGameCharacter::UpdateCameraHeight()
{
    if (!FirstPersonCameraComponent)
    {
        GetWorldTimerManager().ClearTimer(CameraInterpTimerHandle);
        return;
    }

    FVector CurrentLoc = FirstPersonCameraComponent->GetRelativeLocation();

    // 목표치 도달 시 타이머 종료로 최적화
    if (FMath::IsNearlyEqual(CurrentLoc.Z, TargetCameraZ, 0.1f))
    {
        FirstPersonCameraComponent->SetRelativeLocation(FVector(CurrentLoc.X, CurrentLoc.Y, TargetCameraZ));
        GetWorldTimerManager().ClearTimer(CameraInterpTimerHandle);
        return;
    }

    // 부드러운 카메라 높이 전환
    float NewZ = FMath::FInterpTo(CurrentLoc.Z, TargetCameraZ, 0.01f, 1.0f);
    FirstPersonCameraComponent->SetRelativeLocation(FVector(CurrentLoc.X, CurrentLoc.Y, NewZ));
}

// =============================================================
// 5. 피해 처리 및 오디오 (Damage & Sound RPCs)
// =============================================================

float ANoobGameCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (HasAuthority())
    {
        Multicast_PlayHitSound();
    }
    else
    {
        Server_PlayHitSound();
    }
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ANoobGameCharacter::Server_PlayHitSound_Implementation()
{
    Multicast_PlayHitSound();
}

void ANoobGameCharacter::Multicast_PlayHitSound_Implementation()
{
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }
}

void ANoobGameCharacter::Multicast_PlayEndGameAnim_Implementation(bool bIsWinner)
{
    UAnimMontage* MontageToPlay = bIsWinner ? VictoryMontage : DefeatMontage;
    if (MontageToPlay && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay, 1.0f);
    }
}
