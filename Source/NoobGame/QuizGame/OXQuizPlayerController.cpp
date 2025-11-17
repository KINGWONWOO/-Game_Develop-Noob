// OXQuizPlayerController.cpp
#include "OXQuizPlayerController.h"
#include "OXQuizGameMode.h"
#include "OXQuizPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "NoobGame/NoobGameCharacter.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraActor.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraShakeBase.h"

void AOXQuizPlayerController::PlayerReady()
{
    Server_PlayerReady();
}

void AOXQuizPlayerController::RequestPunch(ACharacter* HitCharacter)
{
    Server_RequestPunch(HitCharacter);
}

void AOXQuizPlayerController::RequestPlayPunchMontage()
{
    Server_RequestPlayPunchMontage();
}

void AOXQuizPlayerController::Client_SetUIOnlyInput_Implementation(bool bIsWinner, ECharacterType WinnerType, ACameraActor* EndingCamera)
{
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT %s] Client_SetUIOnlyInput CALLED. bYouWon: %s"), *GetName(), bIsWinner ? TEXT("True") : TEXT("False"));

    // --- [ ⬇️  중요 수정! (Important Fix!) ⬇️ ] ---
    // [수정] 몽타주가 재생되는 동안에는 조작을 막기 위해 'UIOnly'로 설정해야 합니다.
    SetInputMode(FInputModeUIOnly());
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT %s] Input Mode SET TO: UIOnly (Movement DISABLED)"), *GetName()); // <-- ADDED LOG
    // --- [ ⬆️  수정 완료 (Fix Complete) ⬆️ ] ---

    bShowMouseCursor = true;

    // C++  ī޶ մϴ.
    if (EndingCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CLIENT %s] Setting View Target to %s"), *GetName(), *EndingCamera->GetName());
        SetViewTargetWithBlend(EndingCamera, 0.5f); // 0.5 
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[CLIENT %s] EndingCamera reference was NULL!"), *GetName());
    }

    // Ʈ  BP ̺Ʈ ȣ
    OnGameOver.Broadcast(bIsWinner);
    Event_ShowResultsScreen(WinnerType, bIsWinner);
}

void AOXQuizPlayerController::Multicast_PlayHitReaction_Implementation(ACharacter* TargetCharacter, UAnimMontage* MontageToPlay)
{
    if (TargetCharacter && MontageToPlay)
    {
        TargetCharacter->PlayAnimMontage(MontageToPlay);
    }
}

void AOXQuizPlayerController::Multicast_PlayPunchMontage_Implementation(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay)
{
    if (PunchingCharacter && MontageToPlay)
    {
        PunchingCharacter->PlayAnimMontage(MontageToPlay);
    }
}

void AOXQuizPlayerController::Client_SetCameraEffect_Implementation(bool bEnableKnockdownEffect)
{
    ApplyKnockdownCameraEffect(bEnableKnockdownEffect);
}

// RequestPunch (�ǰ� ó��)
bool AOXQuizPlayerController::Server_PlayerReady_Validate() { return true; }
void AOXQuizPlayerController::Server_PlayerReady_Implementation()
{
    AOXQuizGameMode* GM = GetWorld()->GetAuthGameMode<AOXQuizGameMode>();
    if (GM)
    {
        GM->PlayerIsReady(this);
    }
}

bool AOXQuizPlayerController::Server_RequestPunch_Validate(ACharacter* HitCharacter) { return true; }
void AOXQuizPlayerController::Server_RequestPunch_Implementation(ACharacter* HitCharacter)
{
    AOXQuizGameMode* GM = GetWorld()->GetAuthGameMode<AOXQuizGameMode>();
    if (GM && HitCharacter)
    {
        GM->ProcessPunch(this, HitCharacter);
    }
}

// RequestPlayPunchMontage (�ִϸ��̼� ���)
bool AOXQuizPlayerController::Server_RequestPlayPunchMontage_Validate() { return true; }
void AOXQuizPlayerController::Server_RequestPlayPunchMontage_Implementation()
{
    ANoobGameCharacter* MyCharacter = Cast<ANoobGameCharacter>(GetPawn());
    AOXQuizPlayerState* MyPlayerState = GetPlayerState<AOXQuizPlayerState>();
    AOXQuizGameMode* GM = GetWorld()->GetAuthGameMode<AOXQuizGameMode>();

    if (MyCharacter && MyPlayerState && GM)
    {
        bool bIsLeft = MyPlayerState->bIsNextPunchLeft;
        MyPlayerState->bIsNextPunchLeft = !bIsLeft;

        UAnimMontage* MontageToPlay = bIsLeft ? MyCharacter->LeftPunchMontage : MyCharacter->RightPunchMontage;

        GM->ProcessPunchAnimation(MyCharacter, MontageToPlay);
    }
}

void AOXQuizPlayerController::Server_SetupEnding_Implementation(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera)
{
    // [ ]
    UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Executing Server_SetupEnding. bIsWinner: %s"), *GetName(), bIsWinner ? TEXT("True") : TEXT("False"));
    ANoobGameCharacter* MyPawn = Cast<ANoobGameCharacter>(GetPawn());
    if (!MyPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("[SERVER PC %s] ... FAILED to GetPawn()!"), *GetName());
        return;
    }
    // [ ġ] (K.O. ׵  )
    MyPawn->SetRagdollState_Server(false);
    UCharacterMovementComponent* MovementComp = MyPawn->GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
        MovementComp->StopMovementImmediately();
    }
    // ڷƮմϴ.
    bool bTeleportSuccess = MyPawn->TeleportTo(TargetLocation, TargetRotation, false, true);
    UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Teleport attempt result: %s"), *GetName(), bTeleportSuccess ? TEXT("Success") : TEXT("Failed"));

    // Ŭ̾Ʈ ī޶  Ͽ RPC ȣ
    Client_SetUIOnlyInput(bIsWinner, WinnerType, EndingCamera);
    UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Called Client_SetUIOnlyInput."), *GetName()); // <-- ADDED LOG

    ANoobGameCharacter* MyCharacter = Cast<ANoobGameCharacter>(GetPawn());
    AOXQuizPlayerState* MyPlayerState = GetPlayerState<AOXQuizPlayerState>();
    AOXQuizGameMode* GM = GetWorld()->GetAuthGameMode<AOXQuizGameMode>();
    if (MyCharacter && MyPlayerState && GM)
    {
        UAnimMontage* MontageToPlay = bIsWinner ? MyCharacter->VictoryMontage : MyCharacter->DefeatMontage;

        // --- [ ] ---
        if (MontageToPlay)
        {
            UAnimInstance* AnimInstance = MyCharacter->GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] AnimInstance valid. Binding OnMontageEnded..."), *GetName()); // <-- ADDED LOG
                // 몽타주가 끝났을 때 OnEndingMontageEnded 함수가 호출되도록 델리게이트에 바인딩합니다.
                AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &AOXQuizPlayerController::OnEndingMontageEnded);

                // 게임 모드를 통해 몽타주 재생을 요청합니다.
                GM->ProcessPunchAnimation(MyCharacter, MontageToPlay);
                UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Called ProcessPunchAnimation with %s."), *GetName(), *MontageToPlay->GetName()); // <-- ADDED LOG
            }
            else
            {
                // [로그 수정]
                UE_LOG(LogTemp, Error, TEXT("[SERVER PC %s] ... FAILED to GetAnimInstance()! Enabling movement immediately."), *GetName());
                Client_EnableMovementAfterEnding();
            }
        }
        else
        {
            // [로그 수정]
            UE_LOG(LogTemp, Error, TEXT("[SERVER PC %s] MontageToPlay (Victory/Defeat) is NULL! Enabling movement immediately."), *GetName());
            Client_EnableMovementAfterEnding();
        }
    }
}

// [로그 추가] 조작 활성화 함수 호출 확인
void AOXQuizPlayerController::Client_EnableMovementAfterEnding_Implementation()
{
    // --- [ ⬇️  로그 추가 (Logs Added) ⬇️ ] ---
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT %s] Client_EnableMovementAfterEnding FIRED."), *GetName());

    // 1. 입력 모드를 FInputModeGameAndUI로 설정하고, 마우스 커서를 켭니다.
    SetInputMode(FInputModeGameAndUI());
    bShowMouseCursor = true;

    UE_LOG(LogTemp, Warning, TEXT("[CLIENT %s] Input Mode SET TO: GameAndUI (Movement ENABLED)"), *GetName()); // <-- ADDED LOG
}

bool AOXQuizPlayerController::Server_SetupEnding_Validate(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera)
{
    // 특별한 검증 로직이 없다면 true를 반환하여 항상 RPC를 실행하도록 합니다.
    return true;
}

// [로그 추가] 몽타주 종료 델리게이트 호출 확인
void AOXQuizPlayerController::OnEndingMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // --- [ ⬇️  로그 추가 (Logs Added) ⬇️ ] ---
    UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] OnEndingMontageEnded FIRED. bInterrupted: %s"), *GetName(), bInterrupted ? TEXT("True") : TEXT("False"));

    // 몽타주가 중단되지 않고 정상적으로 끝났는지 확인합니다.
    if (!bInterrupted)
    {
        ANoobGameCharacter* MyCharacter = Cast<ANoobGameCharacter>(GetPawn());
        if (MyCharacter)
        {
            // 끝난 몽타주가 우리가 기다리던 승리 또는 패배 몽타주인지 확인합니다.
            bool bIsVictoryMontage = (Montage == MyCharacter->VictoryMontage);
            bool bIsDefeatMontage = (Montage == MyCharacter->DefeatMontage);
            UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Checking Montage... IsVictory: %s, IsDefeat: %s"), *GetName(), bIsVictoryMontage ? TEXT("True") : TEXT("False"), bIsDefeatMontage ? TEXT("True") : TEXT("False"));

            if (bIsVictoryMontage || bIsDefeatMontage)
            {
                // 델리게이트가 중복 호출되지 않도록 바인딩을 해제합니다.
                UAnimInstance* AnimInstance = MyCharacter->GetMesh()->GetAnimInstance();
                if (AnimInstance)
                {
                    AnimInstance->OnMontageEnded.RemoveDynamic(this, &AOXQuizPlayerController::OnEndingMontageEnded);
                    UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Delegate Removed."), *GetName()); // <-- ADDED LOG
                }

                // 해당 클라이언트에게 이동을 다시 활성화하라고 명령합니다.
                Client_EnableMovementAfterEnding();
                UE_LOG(LogTemp, Warning, TEXT("[SERVER PC %s] Calling Client_EnableMovementAfterEnding..."), *GetName()); // <-- ADDED LOG
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[SERVER PC %s] Montage DID NOT MATCH! Movement will NOT be re-enabled."), *GetName()); // <-- ADDED LOG
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[SERVER PC %s] GetPawn() failed inside OnEndingMontageEnded!"), *GetName()); // <-- ADDED LOG
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[SERVER PC %s] Montage was interrupted! Movement will NOT be re-enabled."), *GetName()); // <-- ADDED LOG
    }
}

