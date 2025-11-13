// OXQuizObstacle_2Choice.cpp

#include "OXQuizObstacle_2Choice.h" // 자신의 헤더 (반드시 처음)

// .cpp에서는 전체 헤더 포함
#include "Components/TextRenderComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

AOXQuizObstacle_2Choice::AOXQuizObstacle_2Choice()
{
    EntranceRoots.SetNum(NumEntrances);
    EntranceMeshes.SetNum(NumEntrances);
    EntranceCollisions.SetNum(NumEntrances);
    EntranceAnswerTexts.SetNum(NumEntrances);

    for (int32 i = 0; i < NumEntrances; ++i)
    {
        EntranceRoots[i] = CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("EntranceRoot_%d"), i));
        EntranceRoots[i]->SetupAttachment(RootComponent); // RootComponent는 부모의 것

        EntranceMeshes[i] = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("EntranceMesh_%d"), i));
        EntranceMeshes[i]->SetupAttachment(EntranceRoots[i]);

        EntranceCollisions[i] = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("EntranceCollision_%d"), i));
        EntranceCollisions[i]->SetupAttachment(EntranceRoots[i]);
        EntranceCollisions[i]->SetCollisionProfileName(TEXT("BlockAllDynamic"));

        EntranceAnswerTexts[i] = CreateDefaultSubobject<UTextRenderComponent>(*FString::Printf(TEXT("EntranceAnswerText_%d"), i));
        EntranceAnswerTexts[i]->SetupAttachment(EntranceRoots[i]);
    }
}

void AOXQuizObstacle_2Choice::SetupQuizVisualsAndCollision()
{
    // QuestionText와 CurrentQuizData는 부모(AQuizObstacleBase)의 멤버입니다.
    QuestionText->SetText(CurrentQuizData.Question);

    int32 NumAnswers = CurrentQuizData.Answers.Num();

    if (NumAnswers != NumEntrances)
    {
        UE_LOG(LogTemp, Warning, TEXT("AOXQuizObstacle_2Choice: 퀴즈 데이터의 답 개수(%d)가 2개가 아닙니다!"), NumAnswers);
        return;
    }

    for (int32 i = 0; i < NumEntrances; ++i)
    {
        EntranceAnswerTexts[i]->SetText(CurrentQuizData.Answers[i]);

        if (i == CurrentQuizData.CorrectAnswerIndex)
        {
            EntranceCollisions[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else
        {
            EntranceCollisions[i]->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            EntranceCollisions[i]->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
}