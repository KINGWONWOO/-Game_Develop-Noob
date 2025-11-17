// QuizObstacleBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameTypes.h"// FQuizData 구조체를 알기 위해 포함
#include "QuizObstacleBase.generated.h"

// .h 파일에서는 전방 선언을 사용합니다.
class UTextRenderComponent;

UCLASS(Abstract)
class NOOBGAME_API AQuizObstacleBase : public AActor
{
    GENERATED_BODY()

public:
    AQuizObstacleBase();

    /** [서버] 스폰 시 GameMode가 호출하는 초기화 함수 */
    void InitializeObstacle(const FQuizData& NewQuizData, float NewMoveSpeed);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> QuestionText;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentQuizData)
    FQuizData CurrentQuizData;

    UPROPERTY(Replicated)
    float MoveSpeed;

    UFUNCTION()
    virtual void OnRep_CurrentQuizData();

    /**
     * [핵심] 퀴즈 데이터에 맞춰 텍스트와 콜리전을 설정하는 순수 가상 함수.
     */
    virtual void SetupQuizVisualsAndCollision() PURE_VIRTUAL(AQuizObstacleBase::SetupQuizVisualsAndCollision, );
};