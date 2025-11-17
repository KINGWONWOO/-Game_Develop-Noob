// FruitGameTypes.h

#pragma once

#include "CoreMinimal.h"
#include "GameTypes.generated.h"

// 0. 공용
UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    ECT_None,
    ECT_Cat		UMETA(DisplayName = "Cat"),
    ECT_Dog		UMETA(DisplayName = "Dog")
};

// 1. Fruit Game
UENUM(BlueprintType)
enum class EFruitType : uint8
{
	FT_None		UMETA(DisplayName = "None"),
	FT_Apple	UMETA(DisplayName = "Apple"),
	FT_Banana	UMETA(DisplayName = "Banana"),
	FT_Cherry	UMETA(DisplayName = "Cherry"),
	FT_Orange	UMETA(DisplayName = "Orange")
};

UENUM(BlueprintType)
enum class EFruitGamePhase : uint8
{
	GP_WaitingToStart	UMETA(DisplayName = "WaitingToStart"), // 2명 대기
	GP_Instructions		UMETA(DisplayName = "Instructions"),  // 게임 설명
	GP_Setup			UMETA(DisplayName = "Setup"), // 각자 과일 선택
	GP_SpinnerTurn		UMETA(DisplayName = "SpinnerTurn"), // <-- 신규 단계 추가
	GP_PlayerTurn		UMETA(DisplayName = "PlayerTurn"), // 턴 진행
	GP_GameOver			UMETA(DisplayName = "GameOver") // 게임 종료
};


// 2. OX Quiz Game
UENUM(BlueprintType)
enum class EQuizGamePhase : uint8
{
    GP_WaitingToStart   UMETA(DisplayName = "WaitingToStart"),
    GP_Ready            UMETA(DisplayName = "Ready"),
    GP_Playing          UMETA(DisplayName = "Playing"),
    GP_GameOver         UMETA(DisplayName = "GameOver")
};

USTRUCT(BlueprintType)
struct FQuizData : public FTableRowBase
{
    GENERATED_BODY()

    /** 퀴즈 문제 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz")
    FText Question;

    /** 선택지 배열 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz")
    TArray<FText> Answers;

    /** 정답 인덱스 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz")
    int32 CorrectAnswerIndex;

};