#include "NoobPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraShakeBase.h"
#include "NoobGameModeBase.h"
#include "NoobGame/NoobGameCharacter.h"
#include "NoobPlayerState.h"
#include "NoobLoadingInterface.h"

void ANoobPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		if (MobileControlsWidgetClass)
		{
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
			if (MobileControlsWidget) MobileControlsWidget->AddToPlayerScreen(0);
		}
	}
}

void ANoobPlayerController::Client_ShowLoadingScreen_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (GI->GetClass()->ImplementsInterface(UNoobLoadingInterface::StaticClass()))
		{
			INoobLoadingInterface::Execute_ShowLoading(GI);
		}
	}
}

void ANoobPlayerController::Client_HideLoadingScreen_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (GI->GetClass()->ImplementsInterface(UNoobLoadingInterface::StaticClass()))
		{
			INoobLoadingInterface::Execute_HideLoading(GI);
		}
	}
}


void ANoobPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void ANoobPlayerController::Client_PlayHitCameraShake_Implementation()
{
	PlayPunchCameraShake();
}

void ANoobPlayerController::PlayPunchCameraShake()
{
    // 사용자가 설정을 켰고, 쉐이크 클래스가 지정된 경우에만 실행
    if (bEnableCameraShake && PunchCameraShakeClass && IsLocalController())
    {
        ClientStartCameraShake(PunchCameraShakeClass, 1.0f);
    }
}

void ANoobPlayerController::RequestPunch(ACharacter* HitCharacter) { Server_RequestPunch(HitCharacter); }
void ANoobPlayerController::RequestPlayPunchMontage() { Server_RequestPlayPunchMontage(); }

bool ANoobPlayerController::Server_RequestPunch_Validate(ACharacter* HitCharacter) { return true; }
void ANoobPlayerController::Server_RequestPunch_Implementation(ACharacter* HitCharacter)
{

	if (HitCharacter)
	{
		// 1. 서버 데미지 적용 (가장 핵심 로직!)
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			1.0f,               // 데미지 양
			this,               // 데미지 유발자 (Controller)
			GetPawn(),          // 데미지 원인 액터 (Player Character)
			UDamageType::StaticClass()
		);
	}
	if (ANoobGameModeBase* GM = GetWorld()->GetAuthGameMode<ANoobGameModeBase>())
	{
		GM->ProcessPunch(this, HitCharacter);
	}
}

int ANoobPlayerController::ChangeOpenUICount(bool OpenUI) {
	if(OpenUI) {
		OpenUICount++;
	} else {
		OpenUICount = FMath::Max(0, OpenUICount - 1);
	}
	return OpenUICount;
}

bool ANoobPlayerController::Server_RequestPlayPunchMontage_Validate() { return true; }
void ANoobPlayerController::Server_RequestPlayPunchMontage_Implementation()
{
	ANoobGameCharacter* MyChar = Cast<ANoobGameCharacter>(GetPawn());
	ANoobPlayerState* MyPS = GetPlayerState<ANoobPlayerState>();
	ANoobGameModeBase* GM = GetWorld()->GetAuthGameMode<ANoobGameModeBase>();

	if (MyChar && MyPS && GM)
	{
		// if (MyChar->GetIsDown()) return;

		bool bIsLeft = MyPS->bIsNextPunchLeft;
		MyPS->bIsNextPunchLeft = !bIsLeft;
		UAnimMontage* Montage = bIsLeft ? MyChar->LeftPunchMontage : MyChar->RightPunchMontage;
		GM->ProcessPunchAnimation(MyChar, Montage);
	}
}

void ANoobPlayerController::RequestActorPunch(AActor* TargetActor)
{
	UE_LOG(LogTemp, Display, TEXT("Lobby: RequestActorPunch to %s"), TargetActor ? *TargetActor->GetName() : TEXT("None"));

	if (TargetActor)
	{
		Server_RequestActorPunch(TargetActor);
	}
	// 애니메이션이 비정상적으로 종료되어 Notify가 안 와도, 1초 뒤엔 무조건 공격 가능 상태로 복구
        float SafeExitTime = 1.0f; 
        GetWorldTimerManager().SetTimer(
            MyChar->AttackSafeTimerHandle, 
            MyChar, 
            &ANoobGameCharacter::ResetAttackState, // 캐릭터에 구현된 상태 초기화 함수
            SafeExitTime, 
            false
        );
}

bool ANoobPlayerController::Server_RequestActorPunch_Validate(AActor* TargetActor) { return true; }

void ANoobPlayerController::Server_RequestActorPunch_Implementation(AActor* TargetActor)
{
	if (TargetActor)
	{
		// 1. 서버 데미지 적용 (가장 핵심 로직!)
		UGameplayStatics::ApplyDamage(
			TargetActor,
			1.0f,               // 데미지 양
			this,               // 데미지 유발자 (Controller)
			GetPawn(),          // 데미지 원인 액터 (Player Character)
			UDamageType::StaticClass()
		);

		// 2. 서버 로그 출력
		UE_LOG(LogTemp, Warning, TEXT("Server: Successfully Damaged %s"), *TargetActor->GetName());
	}
}

void ANoobPlayerController::Multicast_PlayHitReaction_Implementation(ACharacter* TargetCharacter, UAnimMontage* MontageToPlay)
{
	if (TargetCharacter && MontageToPlay) TargetCharacter->PlayAnimMontage(MontageToPlay);
}

void ANoobPlayerController::Multicast_PlayPunchMontage_Implementation(ACharacter* PunchingCharacter, UAnimMontage* MontageToPlay)
{
	if (PunchingCharacter && MontageToPlay) PunchingCharacter->PlayAnimMontage(MontageToPlay);
}

void ANoobPlayerController::Client_SetCameraEffect_Implementation(bool bEnableKnockdownEffect)
{
	ApplyKnockdownCameraEffect(bEnableKnockdownEffect);
}

bool ANoobPlayerController::Server_SetupEnding_Validate(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera) { return true; }
void ANoobPlayerController::Server_SetupEnding_Implementation(bool bIsWinner, FVector TargetLocation, FRotator TargetRotation, ECharacterType WinnerType, ACameraActor* EndingCamera)
{
	ANoobGameCharacter* MyPawn = Cast<ANoobGameCharacter>(GetPawn());
	if (!MyPawn) return;

	// 1. 상태 초기화 및 이동 제어
	MyPawn->SetDownState_Server(false);

	if (auto* MoveComp = MyPawn->GetCharacterMovement())
	{
		MoveComp->SetMovementMode(EMovementMode::MOVE_None); // 완전히 움직임 고정
		MoveComp->StopMovementImmediately();
		MoveComp->bOrientRotationToMovement = false; // 이동 방향으로 몸 돌리기 해제
	}

	// 2. 서버 측 순간이동 및 회전 고정
	MyPawn->TeleportTo(TargetLocation, TargetRotation, false, true);
	SetControlRotation(TargetRotation);

	// 3. 클라이언트 동기화 호출 (회전 정보 전달)
	Client_SetUIOnlyInput(bIsWinner, WinnerType, EndingCamera, TargetLocation, TargetRotation);

	// 4. 애니메이션 재생 로직
	ANoobGameModeBase* GM = GetWorld()->GetAuthGameMode<ANoobGameModeBase>();
	UAnimMontage* MontageToPlay = bIsWinner ? MyPawn->VictoryMontage : MyPawn->DefeatMontage;

	if (GM && MontageToPlay)
	{
		if (UAnimInstance* AnimInst = MyPawn->GetMesh()->GetAnimInstance())
		{
			AnimInst->OnMontageEnded.RemoveDynamic(this, &ANoobPlayerController::OnEndingMontageEnded);
			AnimInst->OnMontageEnded.AddUniqueDynamic(this, &ANoobPlayerController::OnEndingMontageEnded);
			GM->ProcessPunchAnimation(MyPawn, MontageToPlay);
		}
		else
		{
			Client_EnableMovementAfterEnding();
		}
	}
	else
	{
		Client_EnableMovementAfterEnding();
	}
}

void ANoobPlayerController::Client_SetUIOnlyInput_Implementation(bool bYouWon, ECharacterType WinnerType, ACameraActor* EndingCamera, FVector TargetLocation, FRotator TargetRotation)
{
	// 1. 입력 모드 변경
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;

	// 네트워크 리플리케이션에 의해 회전값이 되돌아가는 것을 방지합니다.
	if (APawn* MyPawn = GetPawn())
	{
		MyPawn->SetActorLocationAndRotation(TargetLocation, TargetRotation);
		SetControlRotation(TargetRotation);
	}

	// 3. 카메라 액터로 뷰 전환
	if (EndingCamera)
	{
		SetViewTargetWithBlend(EndingCamera, 0.5f);
	}

	OnGameOver.Broadcast(bYouWon);
	Event_ShowResultsScreen(WinnerType, bYouWon);
}

void ANoobPlayerController::Client_EnableMovementAfterEnding_Implementation()
{
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;
}

void ANoobPlayerController::OnEndingMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ANoobGameCharacter* MyPawn = Cast<ANoobGameCharacter>(GetPawn());
	if (MyPawn && (Montage == MyPawn->VictoryMontage || Montage == MyPawn->DefeatMontage))
	{
		if (UAnimInstance* AnimInst = MyPawn->GetMesh()->GetAnimInstance())
		{
			AnimInst->OnMontageEnded.RemoveDynamic(this, &ANoobPlayerController::OnEndingMontageEnded);
		}
		Client_EnableMovementAfterEnding();
	}
}

void ANoobPlayerController::Server_RequestLevelTransition_Implementation(const FString& MapName)
{
	// 서버에서만 작동하므로 GameMode 캐스팅이 성공함
	if (ANoobGameModeBase* GM = GetWorld()->GetAuthGameMode<ANoobGameModeBase>())
	{
		GM->Server_TransitionToSelectedMap(MapName);
	}
	FString TravelURL = FString::Printf(TEXT("%s?listen"), *MapName);
	GetWorld()->ServerTravel(TravelURL);
}


void ANoobPlayerController::RequestLevelTransition(const FString& MapName)
{
	// 서버 RPC 호출
	Server_RequestLevelTransition(MapName);
}
