#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameTypes.h"
#include "InteractableFruitObject.generated.h"

UCLASS()
class NOOBGAME_API AInteractableFruitObject : public AActor
{
	GENERATED_BODY()

public:
	AInteractableFruitObject();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// (신규) 추측 배열에서 이 오브젝트의 인덱스 (0~4). 에디터에서 설정
	UPROPERTY(EditInstanceOnly, Category = "Fruit Game")
	int32 GuessIndex;

	// (신규) 현재 이 오브젝트가 표시하는 과일
	UPROPERTY(ReplicatedUsing = OnRep_CurrentFruit)
	EFruitType CurrentFruit;

	// (신규) 과일 종류에 따라 머티리얼(텍스처)을 변경하기 위한 맵. BP에서 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fruit Game")
	TMap<EFruitType, UMaterialInterface*> FruitMaterials;

	// (신규) 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// (신규) 서버에서 과일 종류를 순환시킴
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Fruit Game")
	EFruitType CycleFruit();

protected:
	// (신규) 클라이언트에서 머티리얼 변경
	UFUNCTION()
	void OnRep_CurrentFruit();

	virtual void BeginPlay() override;
};