// FruitGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FruitGame/FruitGameTypes.h"
#include "GameFramework/PlayerState.h"
#include "FruitGameState.generated.h"

// UI 바인딩을 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFirstTurnPlayerDetermined, int32, StartingPlayerState);

UCLASS()
class NOOBGAME_API AFruitGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFruitGameState();

	// 리플리케이트할 변수들을 등록합니다.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** (수정!) OnRep_GamePhase 함수를 호출하도록 ReplicatedUsing 추가 */
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Game State")
	EGamePhase CurrentGamePhase;

	/** (수정!) OnRep_CurrentActivePlayer 함수를 호출하도록 ReplicatedUsing 추가 */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentActivePlayer, BlueprintReadOnly, Category = "Game State")
	APlayerState* CurrentActivePlayer;

	/** 블루프린트(HUD)가 읽을 수 있도록 UPROPERTY() 추가 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	float ServerTimeAtTurnStart;

	/** Replicated -> ReplicatedUsing (OnRep) 또는 BlueprintReadOnly 추가 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	APlayerState* Winner;

	/** Replicated -> ReplicatedUsing (OnRep) 또는 BlueprintReadOnly 추가 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	ECharacterType WinningCharacterType;

	// 게임 단계가 변경될 때 UI에 알리기 위한 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnFirstTurnPlayerDetermined OnFirstTurnPlayerDetermined;

protected:
	// CurrentGamePhase가 클라이언트에서 복제될 때 호출됩니다.
	UFUNCTION()
	void OnRep_GamePhase();

	// CurrentActivePlayer가 클라이언트에서 복제될 때 호출
	UFUNCTION()
	void OnRep_CurrentActivePlayer();

};