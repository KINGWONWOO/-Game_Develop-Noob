# 🎮 NoobGame (Multiplayer Party Game Project)

![Project Banner](https://via.placeholder.com/1200x400?text=NoobGame+Project+Banner)

> **"다양한 미니게임을 친구들과 함께 즐기는 멀티플레이어 파티 게임"**
>
> *언리얼 엔진 5를 활용한 네트워크 게임 개발 포트폴리오입니다.*

---

## 📋 1. 프로젝트 개요 (Overview)

*   **프로젝트명:** NoobGame
*   **장르:** 멀티플레이어 파티 / 캐주얼 / 서바이벌
*   **개발 인원:** 1인 개발 (원우)
*   **개발 기간:** 202X.XX ~ 진행 중
*   **플랫폼:** PC (Windows)
*   **핵심 목표:**
    *   언리얼 엔진의 **Replication(네트워크 동기화)** 시스템 이해 및 구현.
    *   다양한 장르(퍼즐, 액션, 공포)의 게임 루프 설계.
    *   모듈화된 게임 모드 및 확장 가능한 아키텍처 구축.

---

## 📸 2. 플레이 사진 (Screenshots)

| 로비 (Lobby) | 과일 게임 (Fruit Game) |
| :---: | :---: |
| ![Lobby](https://via.placeholder.com/400x225?text=Lobby+Screenshot) | ![FruitGame](https://via.placeholder.com/400x225?text=FruitGame+Screenshot) |
| **미로 게임 (Maze)** | **퀴즈 게임 (Quiz)** |
| ![Maze](https://via.placeholder.com/400x225?text=Maze+Screenshot) | ![Quiz](https://via.placeholder.com/400x225?text=Quiz+Screenshot) |

---

## 🎥 3. 플레이 영상 (Gameplay Video)

*   *플레이 영상 링크가 여기에 추가될 예정입니다. (YouTube / Vimeo)*

---

## 🛠️ 4. 사용 기술 (Tech Stack)

### Engine & Language
*   **Unreal Engine 5.x**: Core Engine
*   **Blueprints (Visual Scripting)**: 주요 게임 로직 및 UI 구현
*   **C++**: 퍼포먼스 최적화 및 복잡한 시스템 구현 (필요 시)

### Tools
*   **Git / GitHub**: 버전 관리 (LFS 대용량 파일 처리)
*   **Visual Studio 2022**: IDE
*   **Blender**: 3D 모델링 및 애니메이션 리깅 (Cat, Dog, Raccoon 등)

---

## 📚 5. 기술 문서 (Technical Docs)

### 5.1 네트워크 동기화 (Network Replication)
*   **Dedicated Server** 모델을 기반으로 설계.
*   `GameState`에서 전체 게임 상태(남은 시간, 점수 등) 관리.
*   `PlayerState`를 활용하여 개별 플레이어의 점수 및 상태 동기화.

<details>
<summary>💻 네트워크 동기화 예시 코드 (접기/펼치기)</summary>

```cpp
// [예시] 점수 업데이트 함수 (Server Only)
void ANoobPlayerState::AddScore(int32 Amount)
{
    if (HasAuthority())
    {
        Score += Amount;
        OnRep_Score(); // 클라이언트에 변경 알림
    }
}

// [예시] 점수 변경 시 UI 업데이트 (Client)
void ANoobPlayerState::OnRep_Score()
{
    if (IsLocalPlayerController())
    {
        UpdateScoreUI(Score);
    }
}
```
</details>

### 5.2 캐릭터 시스템 (Character System)
*   다양한 동물 캐릭터(Cat, Dog, Raccoon) 지원.
*   공통 부모 클래스(`BP_BaseCharacter`)를 통한 상속 구조 설계.
*   `AnimInstance`를 활용한 상태 기반 애니메이션 제어.

---

## 🎮 6. 구현 기능 및 게임 모드 (Features & Game Modes)

### 6.1 로비 시스템 (Lobby)
*   **기능**: 캐릭터 선택, 채팅, 방 생성 및 입장.
*   **특징**: 직관적인 UI와 실시간 캐릭터 프리뷰 제공.

### 6.2 과일 게임 (Fruit Game)
*   **목표**: 제한 시간 내에 지정된 과일을 모으기.
*   **구현**:
    *   물리 엔진이 적용된 과일 오브젝트 스폰.
    *   `Overlap Event`를 통한 수집 판정.
    *   UI 드래그 앤 드롭 시스템.

<details>
<summary>💻 과일 스폰 로직 (접기/펼치기)</summary>

```cpp
// [Pseudo Code] 랜덤 위치에 과일 생성
void AFruitGameMode::SpawnFruits()
{
    for (int i = 0; i < MaxFruits; i++)
    {
        FVector RandomLoc = GetRandomPointInNavMesh();
        GetWorld()->SpawnActor<AFruitActor>(FruitClass, RandomLoc, FRotator::ZeroRotator);
    }
}
```
</details>

### 6.3 미로 게임 (Maze)
*   **목표**: 복잡한 미로를 탈출하여 목적지에 도달.
*   **특징**:
    *   **Backroom 테마**: 공포 분위기 조성.
    *   **거울 미로**: 반사 재질을 활용한 시각적 트릭.
    *   길 찾기 알고리즘을 활용한 적 AI (추후 구현 예정).

### 6.4 퀴즈 게임 (Quiz)
*   **목표**: O/X 퀴즈를 맞추고 생존하기.
*   **구현**:
    *   `DataTable`을 활용한 문제 관리.
    *   정답/오답 구역 판정 (`BoxComponent`).
    *   탈락자 처리를 위한 `Spectator Mode` 전환.

---

## 📂 7. 프로젝트 구조 (Project Structure)

```
Content/
├── Blueprints/       # 게임 로직
│   ├── Core/         # GameMode, GameState, PlayerController
│   ├── Characters/   # Cat, Dog, Raccoon 등 캐릭터 BP
│   ├── UI/           # Widget Blueprints (HUD, Menu)
│   └── Props/        # 상호작용 오브젝트
├── Levels/           # 맵 (Lobby, Maze, Fruit, Quiz)
├── Models/           # 3D 모델링 리소스 (Blender)
├── Materials/        # 머티리얼 및 텍스처
└── Sounds/           # 효과음 및 BGM
```

---

## 🐛 8. 트러블 슈팅 (Troubleshooting)

### 이슈 1: 대용량 파일(LFS) 업로드 문제
*   **문제**: `.blend` 파일 용량이 100MB를 초과하여 GitHub Push 실패.
*   **해결**: `Git LFS`를 도입하여 대용량 파일 추적 관리하거나, 불필요한 백업 파일(`.blend1`)을 제외하여 최적화.

### 이슈 2: 멀티플레이어 위치 동기화 지연
*   **문제**: 클라이언트 간 캐릭터 위치가 뚝뚝 끊기는 현상(Jittering) 발생.
*   **해결**: `CharacterMovementComponent`의 네트워크 보정 설정 튜닝 및 `Replication Rate` 조절로 해결.

---

## 🚀 9. 향후 계획 (Future Updates)

*   [ ] **AI 고도화**: 미로 게임 내 추격자 AI 구현 (Behavior Tree 활용).
*   [ ] **상점 시스템**: 게임 재화로 꾸미기 아이템 구매 기능.
*   [ ] **최적화**: 텍스처 아틀라스 및 LOD 적용으로 퍼포먼스 개선.
*   [ ] **모바일 이식**: 터치 인터페이스 지원 및 최적화.

---

*Contact: Wonwoo (Email or Link)*
