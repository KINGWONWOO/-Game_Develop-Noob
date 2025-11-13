// OXQuizObstacle_3Choice.h

#pragma once

#include "CoreMinimal.h"
#include "QuizObstacleBase.h"
#include "OXQuizObstacle_3Choice.generated.h"

// .h에서는 전방 선언
class UBoxComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class USceneComponent;

UCLASS()
class NOOBGAME_API AOXQuizObstacle_3Choice : public AQuizObstacleBase
{
    GENERATED_BODY()

public:
    AOXQuizObstacle_3Choice();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<USceneComponent>> EntranceRoots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UStaticMeshComponent>> EntranceMeshes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UBoxComponent>> EntranceCollisions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UTextRenderComponent>> EntranceAnswerTexts;

    /** 부모의 순수 가상 함수를 구현(Override)합니다. */
    virtual void SetupQuizVisualsAndCollision() override;

private:
    const int32 NumEntrances = 3;
};