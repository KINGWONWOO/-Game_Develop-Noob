// OXQuizGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OXQuizTypes.h" // EQuizGamePhase를 알기 위해 포함
#include "OXQuizGameState.generated.h"

class APlayerState;

UCLASS()
class NOOBGAME_API AOXQuizGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AOXQuizGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** [서버] 현재 게임 페이즈를 설정합니다. */
    void SetCurrentPhase(EQuizGamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Game")
    EQuizGamePhase GetCurrentPhase() const { return CurrentPhase; }

    /** [서버] 승자와 패자를 설정합니다. */
    UFUNCTION(Server, Reliable)
    void Server_SetWinnerAndLoser(APlayerState* NewWinner, APlayerState* NewLoser);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Game")
    TObjectPtr<APlayerState> Winner;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Game")
    TObjectPtr<APlayerState> Loser;

protected:
    /**
     * @brief 현재 게임 페이즈 (모든 클라이언트에 복제됨)
     */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase)
    EQuizGamePhase CurrentPhase;

    /**
     * @brief 클라이언트에서 페이즈 변경 시 호출되는 RepNotify
     */
    UFUNCTION()
    void OnRep_CurrentPhase();
};