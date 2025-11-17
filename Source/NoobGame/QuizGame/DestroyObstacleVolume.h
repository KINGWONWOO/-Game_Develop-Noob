// DestroyObstacleVolume.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestroyObstacleVolume.generated.h"

class UBoxComponent;

UCLASS()
class NOOBGAME_API ADestroyObstacleVolume : public AActor
{
    GENERATED_BODY()

public:
    ADestroyObstacleVolume();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> CollisionComponent;

    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};