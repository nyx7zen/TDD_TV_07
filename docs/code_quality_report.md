# TDD TV Channel Controller — 코드 품질 분석 (SOLID / Code Smell)

| 항목 | 내용 |
|------|------|
| 분석 대상 | `TVChannelController`, `FakeTuner`, `ITuner` (연관 테스트 더블) |
| 기준 | `README.md`, `docs/requirements_analysis.md`, C++17 |
| 코드 상태 | `TVChannelController.cpp` 스텁, `FakeTuner` 부분 구현 |
| 관점 | 시니어 C++ 아키텍트 + 모던 C++ 리뷰어 |

---

## 1. SOLID / 책임 혼재 분석

### 1.1 SRP (Single Responsibility Principle)

`TVChannelController` 한 클래스에 다음 책임이 **설계상** 모두 배치되어 있다.

| 책임 영역 | 관련 멤버·메서드 | SRP 관점 |
|-----------|------------------|----------|
| 숫자 입력 버퍼 | `inputBuffer_`, `pressNumber`, `pressConfirm`, `pressOther` | 입력 상태 머신 — 별도 `ChannelInputBuffer` 후보 |
| 채널 적용·유효성 | `isValidChannel`, `applyChannel`, `tuner_.setCH` | 튜너 위임 전 정책 — `applyChannel` 파이프라인으로 응집 가능 |
| 선호 채널 CRUD | `favorites_`, `isFavorite`, `addFavorite`, `pressFavorite` | 목록 관리 — `FavoriteChannelList` 후보 |
| 다음 선호 탐색 | `pressNextFavorite` (+ 예정 `findNextFavorite`) | 정렬 벡터 + `upper_bound` 알고리즘 |
| 리모컨 API 표면 | public `press*` 메서드 5개 | **Facade** 역할은 적절하나, 내부 구현이 비대해지면 SRP 위반 심화 |

**판정:** 구현 전 단계에서도 헤더·README 리팩토링 TODO가 **다중 책임 분리**를 전제한다. 기능 Green 이후 SRP 위반이 실질적 코드 스멜로 드러난다.

### 1.2 OCP (Open/Closed Principle)

| 지점 | 내용 | OCP 평가 |
|------|------|----------|
| `ITuner&` 주입 | 실제 튜너 / Fake / Mock 교체 가능 | ✅ 확장에 열림, 수정에 닫힘 (인터페이스 안정) |
| `pressNumber` 등 고정 API | 새 버튼 타입마다 메서드 추가 | ⚠️ README 범위 내에서는 수용; 버튼·명령이 늘면 Command/Handler 맵 고려 |
| 채널 범위 `0~99` 하드코딩 | 지역·모델별 채널 상한 변경 시 컨트롤러·FakeTuner 동시 수정 | ⚠️ `ChannelPolicy` 상수/설정 객체로 한곳화 필요 |
| `ITuner.h` 수정 불가 | 외부 계약 고정 | 의도적 제약 — OCP는 **구현체 교체**로만 확장 |

**판정:** 튜너 추상화는 OCP에 유리하나, **채널 정책·입력 규칙**이 클래스 전역에 흩어지면 OCP·DRY 모두 약화된다.

### 1.3 기타 SOLID (요약)

| 원칙 | 현황 |
|------|------|
| LSP | `FakeTuner` / `MockTuner`가 `ITuner` 계약 준수 — `setCH` 예외·문자열 형식 일치 필요 |
| ISP | `ITuner` 3메서드 — 컨트롤러는 `seekCH` 미사용이나 인터페이스는 과제 제공 범위로 적절 |
| DIP | `TVChannelController` → `ITuner&` 의존 ✅ 구체 튜너에 의존하지 않음 |

---

## 2. Magic Number 상수화 필요성

| 매직 값 | 출현 위치 | 권장 상수·이름 | 우선순위 |
|---------|-----------|----------------|----------|
| `-1` | `inputBuffer_` 초기값·Idle | `kEmptyInput` / `ChannelInputBuffer::empty()` | 높음 |
| `0`, `99` | `isValidChannel`, `FakeTuner::setCH` | `kMinChannel`, `kMaxChannel` (공유 `namespace ChannelLimits`) | 높음 |
| `10` | 두 자리 조합 `inputBuffer_ * 10 + digit` | `kDecimalBase` 또는 버퍼 클래스 캡슐화 | 중간 |
| `0~9` | `pressNumber` digit 검증 | `kMinDigit`, `kMaxDigit` | 중간 |
| `99` vs `100` | 예외 경계 | `kMaxChannel + 1` 문서화 | 중간 |

**중복 위험:** `FakeTuner`와 `TVChannelController::isValidChannel`이 **동일 범위 규칙**을 각각 표현 → DRY 위반·테스트 불일치 가능. Green 후 `applyChannel` 단일 경로 + FakeTuner는 `setCH` 위임으로 정렬하거나, 공유 `ChannelPolicy::isValid(int)` 헤더로 추출한다.

---

## 3. Code Smell 분석 표

| 문제점 | 위반 원칙 / 스멜 | 영향 | 개선 방향 | 우선순위 |
|--------|------------------|------|-----------|----------|
| `TVChannelController`에 입력·즐겨찾기·튜너 호출 혼재 | **SRP**, God Class 전조 | 메서드·테스트 비대, 변경 파급 | `ChannelInputBuffer`, `FavoriteChannelList` 추출 (README refactor) | **1** |
| `applyChannel` 스텁·채널 적용 경로 미정립 | **Duplicated Code** 예정, Shotgun Surgery | `pressNumber`/`pressConfirm`/`pressNextFavorite` 각각 `setCH` 호출 분산 위험 | 유효성 검사 → `tuner_.setCH(to_string(ch))` 단일 파이프라인 | **1** |
| `inputBuffer_ = -1` 반복 (예정 3곳+) | **Duplicated Code** | 클리어 누락 버그 | `clearBuffer()` 추출 (README) | **2** |
| `addFavorite` vs `pressFavorite` 추가 로직 | **Duplicated Code** | 테스트 헬퍼와 프로덕션 경로 불일치 | `addToFavorites()` / `removeFromFavorites()` 내부화 | **2** |
| `isValidChannel` vs `FakeTuner` 범위 검사 | **Duplicated Code**, **Feature Envy** (정책이 튜너에도 존재) | 100 채널 예외 동작 불일치 | 컨트롤러 선검증 + Fake는 최종 방어, 또는 공유 Policy | **2** |
| `pressNextFavorite` 내 `upper_bound`+wrap 인라인 예상 | **Long Method**, **복잡 조건** | 가독성·단위 테스트 어려움 | `findNextFavorite(int current) const` 추출 (README) | **3** |
| 헤더 인라인 `addFavorite` + `std::sort` | **응집도 혼재** (선언부 비대) | 컴파일 의존·재사용 제한 | 선언만 헤더, 구현 `.cpp` 또는 `FavoriteChannelList` | **3** |
| `isFavorite` 선형 탐색 | 성능 스멜 (과제 규모에선 미미) | 목록 커지면 비용 | `std::binary_search` (정렬 유지 전제) 또는 `unordered_set` | **4** |
| public `addFavorite` (테스트 전용 성격) | **API 오염** | 프로덕션 계약 모호 | 테스트 픽스처·friend 또는 `FavoriteChannelList` 빌더 | **4** |
| `FakeTuner::seekCH` 빈 `available_` 미방어 | **잠재 결함** | UB (`front()`) | 생성자 assert/throw 또는 문서화된 전제 | **5** |
| `TVChannelController.cpp` 전 메서드 빈 구현 | **Dead Code** / 미완성 | README 기능 0% | TDD로 `pressNumber` → `applyChannel` 순 구현 | **1** (기능) |

---

## 4. C++17 스타일·구조 개선 방향

### 4.1 책임 분리 (README dev/refactoring 정합)

| 추출 대상 | 역할 | README 항목 |
|-----------|------|-------------|
| `ChannelInputBuffer` | `-1` 센티넬, 한·두 자리 입력, `clear()` | `clearBuffer()` |
| `FavoriteChannelList` | 정렬 유지, toggle, `upper_bound` 기반 next | `addToFavorites()`, `findNextFavorite()` |
| `applyChannel(int)` | `isValidChannel` + `tuner_.setCH` | 유효성 → 튜너 단일 책임 |

### 4.2 알고리즘·API

- `pressNextFavorite`: `std::upper_bound(favorites_.begin(), favorites_.end(), current)` + wrap → `favorites_.front()` (requirements T-19~T-23).
- digit 검증: `if (digit < kMinDigit || digit > kMaxDigit) return;` — early return으로 중첩 감소.
- `constexpr` 채널 상한: C++17 `inline constexpr int kMaxChannel = 99;` in `ChannelLimits.h`.

### 4.3 Command / Handler (중장기)

새 리모컨 버튼 추가 시 OCP 강화:

```cpp
// 방향성 예시 — Green 이후 선택적
using ButtonHandler = std::function<void(TVChannelController&)>;
std::unordered_map<ButtonId, ButtonHandler> handlers_;
```

과제 범위에서는 **5개 `press*` 메서드**로 충분; 리팩토링 브랜치 이후 필요 시 도입.

---

## 5. Test Double 설계와의 관계

| 구분 | 설계 요구 | 코드 품질 연계 |
|------|-----------|----------------|
| `ITuner&` DI | 컨트롤러가 구체 타입 비의존 | SRP·DIP 유지의 전제 — **유지 필수** |
| FakeTuner | 상태 검증 (`getCurrentCH`, favorites) | 채널 범위 로직 **중복** 시 Fake·Controller 테스트가 서로 다른 “진실”을 가질 수 있음 → `applyChannel` 단일화 |
| MockTuner | `setCH`/`getCurrentCH` 호출·순서 | `applyChannel`이 유일한 `setCH` 진입점이면 Mock 기대가 단순해짐 |
| 검증 경계 | Fake = 결과, Mock = 협력 | 컨트롤러 내부에 `tuner_` 직접 호출이 여러 곳이면 Mock 테스트 취약 → **applyChannel 집중** |

---

## 6. 리팩토링 우선순위 (1~5) — Green 테스트 유지 전제

| 순위 | 작업 | 이유 |
|------|------|------|
| **1** | TDD Green: `applyChannel` → `pressNumber` / `pressConfirm` / `pressOther` / `pressFavorite` / `pressNextFavorite` | 스텁 제거·기능 없으면 리팩토링 근거 없음; README 28 시나리오 충족이 최우선 |
| **2** | `clearBuffer()`, `applyChannel` 파이프라인 정리, `isValidChannel` 내부화 | README refactor 직결; 중복·버퍼 버그 예방 |
| **3** | `findNextFavorite()`, `addToFavorites()` / toggle 통합 | `upper_bound` 복잡도 격리; `addFavorite` vs `pressFavorite` 중복 제거 |
| **4** | Magic Number → `ChannelLimits`, `ChannelInputBuffer` 클래스 추출 | 가독성·DRY; dev/refactoring 브랜치 목표와 일치 |
| **5** | 테스트 픽스처·Given-When-Then·lcov 80%+ | 품질 게이트; 구조 안정 후 커버리지·가독성 |

---

## 7. 개선 방향 요약

1. **단기 (dev):** `applyChannel`을 유일한 채널 변경·예외 관문으로 구현하고, README T-01~T-28을 Green으로 만든다.
2. **중기 (dev/refactoring):** 입력 버퍼·선호 목록·다음 채널 탐색을 각각 메서드/클래스로 추출해 SRP를 회복한다. README의 `clearBuffer`, `findNextFavorite`, `addToFavorites` 항목과 1:1 대응시킨다.
3. **상수·정책:** `0`/`99`/`-1`/`10`을 named constant 또는 작은 Policy 타입으로 모아 `FakeTuner`와 컨트롤러가 동일 규칙을 공유한다.
4. **Test Double:** `ITuner&` DI는 유지한다. Fake는 관찰 가능한 상태, Mock은 `applyChannel` 경유 `setCH`/`getCurrentCH` 계약만 검증한다.
5. **OCP·Command:** 과제 범위에서는 5개 `press*` API로 충분하다. 버튼·기능 확장 시 Command 맵을 검토한다.

현재 코드베이스는 **구조 의도(헤더·인터페이스)는 양호**하나, **구현 스텁**으로 인해 SRP·DRY 이슈는 “예정된 부채” 상태이다. Green 달성 직후 우선순위 2~3 리팩토링을 적용하면 README refactor TODO와 요구사항 분석서가 정합된다.
