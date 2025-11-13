// OXQuizObstacle_2Choice.h

#pragma once

#include "CoreMinimal.h"
#include "QuizObstacleBase.h"
#include "OXQuizObstacle_2Choice.generated.h"

// .h에서는 전방 선언
class UBoxComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class USceneComponent;

UCLASS()
class NOOBGAME_API AOXQuizObstacle_2Choice : public AQuizObstacleBase
{
    GENERATED_BODY()

public:
    AOXQuizObstacle_2Choice();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<USceneComponent>> EntranceRoots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UStaticMeshComponent>> EntranceMeshes; // 벽

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UBoxComponent>> EntranceCollisions; // 막는 콜리전

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UTextRenderComponent>> EntranceAnswerTexts; // O, X

    /** 부모의 순수 가상 함수를 구현(Override)합니다. */
    virtual void SetupQuizVisualsAndCollision() override;

private:
    const int32 NumEntrances = 2;
};