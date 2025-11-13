// DeadZoneVolume.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeadZoneVolume.generated.h"

class UBoxComponent;
class AOXQuizGameMode;

UCLASS()
class NOOBGAME_API ADeadZoneVolume : public AActor
{
    GENERATED_BODY()

public:
    ADeadZoneVolume();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> CollisionComponent;

    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY()
    TObjectPtr<AOXQuizGameMode> CachedGameMode;
};