// QuizObstacleBase.cpp

#include "QuizObstacleBase.h"         // 자신의 헤더 (가장 먼저)
#include "Components/TextRenderComponent.h" // .cpp에서는 전체 헤더 포함
#include "Net/UnrealNetwork.h"        // DOREPLIFETIME, HasAuthority 등을 위해 포함
#include "Components/SceneComponent.h"

AQuizObstacleBase::AQuizObstacleBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = Root;

    QuestionText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("QuestionText"));
    QuestionText->SetupAttachment(RootComponent);

    MoveSpeed = 0.0f;
}

void AQuizObstacleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AQuizObstacleBase, CurrentQuizData);
    DOREPLIFETIME(AQuizObstacleBase, MoveSpeed);
}

void AQuizObstacleBase::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority() && !CurrentQuizData.Question.IsEmpty())
    {
        SetupQuizVisualsAndCollision();
    }
}

void AQuizObstacleBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector Location = GetActorLocation();
    Location += GetActorForwardVector() * MoveSpeed * DeltaTime;
    SetActorLocation(Location);
}

void AQuizObstacleBase::InitializeObstacle(const FQuizData& NewQuizData, float NewMoveSpeed)
{
    if (HasAuthority())
    {
        CurrentQuizData = NewQuizData;
        MoveSpeed = NewMoveSpeed;
        SetupQuizVisualsAndCollision();
    }
}

void AQuizObstacleBase::OnRep_CurrentQuizData()
{
    SetupQuizVisualsAndCollision();
}