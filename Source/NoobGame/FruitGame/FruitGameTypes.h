// FruitGameTypes.h

#pragma once

#include "CoreMinimal.h"
#include "FruitGameTypes.generated.h"

// 1. 과일 종류 Enum
UENUM(BlueprintType)
enum class EFruitType : uint8
{
	FT_None		UMETA(DisplayName = "None"),
	FT_Apple	UMETA(DisplayName = "Apple"),
	FT_Banana	UMETA(DisplayName = "Banana"),
	FT_Cherry	UMETA(DisplayName = "Cherry"),
	FT_Orange	UMETA(DisplayName = "Orange")
};

// 2. 게임 상태 Enum
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	GP_WaitingToStart	UMETA(DisplayName = "WaitingToStart"), // 2명 대기
	GP_Instructions		UMETA(DisplayName = "Instructions"),  // 게임 설명
	GP_Setup			UMETA(DisplayName = "Setup"), // 각자 과일 선택
	GP_SpinnerTurn		UMETA(DisplayName = "SpinnerTurn"), // <-- 신규 단계 추가
	GP_PlayerTurn		UMETA(DisplayName = "PlayerTurn"), // 턴 진행
	GP_GameOver			UMETA(DisplayName = "GameOver") // 게임 종료
};