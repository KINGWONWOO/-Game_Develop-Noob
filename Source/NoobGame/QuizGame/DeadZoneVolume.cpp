// DeadZoneVolume.cpp

#include "DeadZoneVolume.h"
#include "Components/BoxComponent.h"
#include "OXQuizGameMode.h"
#include "NoobGameCharacter.h" // [수정] 기존 캐릭터 헤더 포함

ADeadZoneVolume::ADeadZoneVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionComponent->SetGenerateOverlapEvents(true);
}

void ADeadZoneVolume::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ADeadZoneVolume::OnOverlapBegin);
        CachedGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOXQuizGameMode>() : nullptr;
    }
}

void ADeadZoneVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    // [수정] ANoobGameCharacter로 캐스팅
    ANoobGameCharacter* PlayerCharacter = Cast<ANoobGameCharacter>(OtherActor);
    if (PlayerCharacter)
    {
        if (CachedGameMode)
        {
            CachedGameMode->HandlePlayerDeath(PlayerCharacter->GetController());
        }
        else
        {
            AOXQuizGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AOXQuizGameMode>() : nullptr;
            if (GM)
            {
                CachedGameMode = GM;
                CachedGameMode->HandlePlayerDeath(PlayerCharacter->GetController());
            }
        }
    }
}