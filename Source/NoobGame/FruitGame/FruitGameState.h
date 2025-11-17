#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameTypes.h"
#include "GameFramework/PlayerState.h"
#include "FruitGameState.generated.h"

// ──────────────────────────────────────────────────────────────────────────
// Delegate Declarations
// ──────────────────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EFruitGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFirstTurnPlayerDetermined, int32, StartingPlayerState);

UCLASS()
class NOOBGAME_API AFruitGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	// ──────────────────────────────────────────────────────────────────────────
	// Constructor & Framework Overrides
	// ──────────────────────────────────────────────────────────────────────────
	AFruitGameState();

	// 리플리케이트할 변수들을 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ──────────────────────────────────────────────────────────────────────────
	// Replicated Properties (Networked State)
	// ──────────────────────────────────────────────────────────────────────────
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Game State")
	EFruitGamePhase CurrentGamePhase;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentActivePlayer, BlueprintReadOnly, Category = "Game State")
	APlayerState* CurrentActivePlayer;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	float ServerTimeAtTurnStart;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	APlayerState* Winner;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	ECharacterType WinningCharacterType;

	// ──────────────────────────────────────────────────────────────────────────
	// Delegates (UI Binding)
	// ──────────────────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnFirstTurnPlayerDetermined OnFirstTurnPlayerDetermined;

protected:
	// ──────────────────────────────────────────────────────────────────────────
	// Replication Notifies (OnRep Functions)
	// ──────────────────────────────────────────────────────────────────────────

	// CurrentGamePhase가 클라이언트에서 복제될 때 호출
	UFUNCTION()
	void OnRep_GamePhase();

	// CurrentActivePlayer가 클라이언트에서 복제될 때 호출
	UFUNCTION()
	void OnRep_CurrentActivePlayer();
};