// OXQuizObstacle_3Choice.cpp

#include "OXQuizObstacle_3Choice.h" // 자신의 헤더 (반드시 처음)

// .cpp에서는 전체 헤더 포함
#include "Components/TextRenderComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

AOXQuizObstacle_3Choice::AOXQuizObstacle_3Choice()
{
    EntranceRoots.SetNum(NumEntrances);
    EntranceMeshes.SetNum(NumEntrances);
    EntranceCollisions.SetNum(NumEntrances);
    EntranceAnswerTexts.SetNum(NumEntrances);

    for (int32 i = 0; i < NumEntrances; ++i)
    {
        EntranceRoots[i] = CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("EntranceRoot_%d"), i));
        EntranceRoots[i]->SetupAttachment(RootComponent);

        EntranceMeshes[i] = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("EntranceMesh_%d"), i));
        EntranceMeshes[i]->SetupAttachment(EntranceRoots[i]);

        EntranceCollisions[i] = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("EntranceCollision_%d"), i));
        EntranceCollisions[i]->SetupAttachment(EntranceRoots[i]);
        EntranceCollisions[i]->SetCollisionProfileName(TEXT("BlockAllDynamic"));

        EntranceAnswerTexts[i] = CreateDefaultSubobject<UTextRenderComponent>(*FString::Printf(TEXT("EntranceAnswerText_%d"), i));
        EntranceAnswerTexts[i]->SetupAttachment(EntranceRoots[i]);
    }
}

void AOXQuizObstacle_3Choice::SetupQuizVisualsAndCollision()
{
    // QuestionText와 CurrentQuizData는 부모(AQuizObstacleBase)의 멤버입니다.
    QuestionText->SetText(CurrentQuizData.Question);

    int32 NumAnswers = CurrentQuizData.Answers.Num();

    if (NumAnswers != NumEntrances)
    {
        UE_LOG(LogTemp, Warning, TEXT("AOXQuizObstacle_3Choice: 퀴즈 데이터의 답 개수(%d)가 3개가 아닙니다!"), NumAnswers);
        return;
    }

    for (int32 i = 0; i < NumEntrances; ++i)
    {
        EntranceAnswerTexts[i]->SetText(CurrentQuizData.Answers[i]);

        if (i == CurrentQuizData.CorrectAnswerIndex)
        {
            EntranceCollisions[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 정답
        }
        else
        {
            EntranceCollisions[i]->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 오답
            EntranceCollisions[i]->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
}