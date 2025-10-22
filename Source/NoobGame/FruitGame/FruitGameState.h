// FruitGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FruitGameTypes.h"
#include "FruitGameState.generated.h"

// UI 바인딩을 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);

UCLASS()
class NOOBGAME_API AFruitGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFruitGameState();

	// 리플리케이트할 변수들을 등록합니다.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 현재 게임 단계 (모두가 알아야 함)
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase)
	EGamePhase CurrentGamePhase;

	// 현재 턴인 플레이어 (모두가 알아야 함)
	UPROPERTY(Replicated)
	APlayerState* CurrentActivePlayer;

	// 남은 턴 시간 (모두가 알아야 함)
	UPROPERTY(Replicated)
	float TurnTimer;

	// 승자 (모두가 알아야 함)
	UPROPERTY(Replicated)
	APlayerState* Winner;

	// 게임 단계가 변경될 때 UI에 알리기 위한 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGamePhaseChanged OnGamePhaseChanged;

protected:
	// CurrentGamePhase가 클라이언트에서 복제될 때 호출됩니다.
	UFUNCTION()
	void OnRep_GamePhase();
};