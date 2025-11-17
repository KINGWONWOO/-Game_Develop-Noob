// DestroyObstacleVolume.cpp

#include "DestroyObstacleVolume.h"
#include "Components/BoxComponent.h"
#include "QuizObstacleBase.h" // [중요] 장애물의 부모 클래스를 포함합니다.

ADestroyObstacleVolume::ADestroyObstacleVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionComponent->SetGenerateOverlapEvents(true);
}

void ADestroyObstacleVolume::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 오버랩 이벤트를 감지합니다.
    if (HasAuthority())
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ADestroyObstacleVolume::OnOverlapBegin);
    }
}

void ADestroyObstacleVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    // 오버랩된 액터가 AQuizObstacleBase (또는 그 자식 클래스)인지 확인합니다.
    AQuizObstacleBase* Obstacle = Cast<AQuizObstacleBase>(OtherActor);
    if (Obstacle)
    {
        // 장애물이 맞다면 즉시 파괴합니다.
        // Destroy()는 서버에서 호출되면 자동으로 모든 클라이언트에도 복제됩니다.
        Obstacle->Destroy();
    }
}