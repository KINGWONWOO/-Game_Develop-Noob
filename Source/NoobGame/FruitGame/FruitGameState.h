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

	// 리플리케이트할 변수들을 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Game State")
	EGamePhase CurrentGamePhase;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentActivePlayer, BlueprintReadOnly, Category = "Game State")
	APlayerState* CurrentActivePlayer;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	float ServerTimeAtTurnStart;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	APlayerState* Winner;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	ECharacterType WinningCharacterType;

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnFirstTurnPlayerDetermined OnFirstTurnPlayerDetermined;

protected:
	// CurrentGamePhase가 클라이언트에서 복제될 때 호출
	UFUNCTION()
	void OnRep_GamePhase();

	// CurrentActivePlayer가 클라이언트에서 복제될 때 호출
	UFUNCTION()
	void OnRep_CurrentActivePlayer();

};