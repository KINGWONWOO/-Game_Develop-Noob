// OXQuizTypes.h

#pragma once

#include "CoreMinimal.h"
#include "OXQuizTypes.generated.h"

/**
 * @brief 게임의 현재 진행 단계를 나타내는 열거형
 */
UENUM(BlueprintType)
enum class EQuizGamePhase : uint8
{
    WaitingToStart,
    Ready,
    Playing,
    GameOver
};

/**
 * @brief 퀴즈 데이터 구조체 (O/X, 객관식 공용)
 */
USTRUCT(BlueprintType)
struct FQuizData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz")
    FText Question;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz")
    TArray<FText> Answers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz")
    int32 CorrectAnswerIndex;

    FQuizData() : CorrectAnswerIndex(0) {}
};