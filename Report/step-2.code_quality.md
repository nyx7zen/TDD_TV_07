# TDD TV Channel Controller — 코드 품질 분석 (SOLID / Code Smell)

| 항목 | 내용 |
|------|------|
| 단계 | Step 2 — 코드 품질 분석 |
| 분석 대상 | `TVChannelController`, `FakeTuner`, `ITuner` (연관 `MockTuner`) |
| 기준 문서 | `README.md`, `Report/step-1.requirements_analysis.md` |
| 기술 스택 | C++17, CMake, Google Test / GMock |
| 코드 상태 | `FakeTuner` 구현 완료, `TVChannelController.cpp` **전 메서드 스텁** |
| 관점 | 시니어 C++ 아키텍트 + 모던 C++ 리뷰어 |
| 작성일 | 2026-05-19 |
| 동기 문서 | `docs/code_quality_report.md` |

---

## 1. 핵심 진단 (Code Smell 표)

| 문제점 | 위반 원칙 / 스멜 | 영향 | 개선 방향 | 우선순위 |
|--------|------------------|------|-----------|----------|
| `TVChannelController`에 **숫자 입력 버퍼**·**채널 적용**·**선호 채널 토글·이동**·**ITuner 위임**이 한 클래스에 공존 | **SRP**, God Class 전조 | `pressNumber` 변경이 `pressNextFavorite` 회귀를 유발; 테스트·리뷰 비대 | `ChannelInputBuffer`, `FavoriteChannelList` 추출 후 컨트롤러는 Facade만 담당 (README dev/refactoring) | **1** |
| `applyChannel`·`press*` 전부 **빈 스텁** — 채널 변경 단일 경로 미정립 | 미완성 / **Shotgun Surgery** 예정 | README T-07~T-28 0% 충족; `setCH` 호출이 메서드마다 흩어질 위험 | TDD Green: `applyChannel` = `isValidChannel` → `tuner_.setCH(to_string(ch))` 단일 파이프라인 | **1** |
| `inputBuffer_ = -1` 클리어가 `pressOther`·`pressConfirm`·두 자리 완성 등 **3곳 이상** 반복될 설계 | **Duplicated Code** | 버퍼 미초기화·이중 적용 버그 (T-10 등) | `clearBuffer()` 추출 — README refactor 1순위 | **2** |
| `isValidChannel(0~99)` vs `FakeTuner::setCH` 범위 검사 **이중 구현** | **DRY**, **Duplicated Code** | 컨트롤러·Fake가 다른 “진실”이면 T-14 vs T-03/04 불일치 | `ChannelLimits` 공유 상수 + 컨트롤러 선검증, Fake는 방어적 검증 유지 | **2** |
| `addFavorite`(public) vs `pressFavorite()` 추가·정렬 로직 **중복 예정** | **Duplicated Code** | Arrange API와 프로덕션 경로 불일치 (T-15~T-18) | `addToFavorites()` / `removeFromFavorites()` 내부화, 테스트는 `pressFavorite` 또는 빌더 사용 | **2** |
| `pressNextFavorite`에 `upper_bound` + wrap-around **인라인** 예상 | **Long Method**, 복잡 조건 | T-19~T-23 가독성·단위 검증 어려움 | `findNextFavorite(int current) const` 추출 (README) | **3** |
| 헤더에 `addFavorite` **인라인 구현** (`push_back` + `sort`) | 응집도 혼재, 헤더 비대 | 컴파일 의존 증가, 선호 목록 책임이 컨트롤러에 고정 | 선언만 헤더 / 구현 `.cpp` 또는 `FavoriteChannelList` 클래스 | **3** |
| `isFavorite` — 정렬된 `favorites_`에 `std::find` **선형 탐색** | 성능 스멜 (과제 규모에선 경미) | 목록 확장 시 비용 | `std::binary_search` (정렬 불변식 유지 전제) | **4** |
| public `addFavorite(int)` — 테스트 Arrange 성격 API | **API 오염** | 프로덕션 계약 모호 | 픽스처 헬퍼, `FavoriteChannelList` friend, 또는 시퀀스만 `pressFavorite`로 구성 | **4** |
| 새 버튼 추가 시 `pressXxx()` 메서드·분기 **직접 확장** | **OCP** (버튼·정책 축) | README 범위 밖 기능 추가 시 컨트롤러 전면 수정 | Green 후 선택: `ButtonId` → `std::function` / Command 맵 (과제 5버튼은 현 API 유지 가능) | **4** |
| `FakeTuner::seekCH` — `available_` 빈 벡터 시 `front()` **UB** | 잠재 결함 | T-06 외 비정상 Arrange 시 크래시 | 생성자 `throw`/`assert`, 또는 빈 목록 방어 + 문서화 | **5** |
| `TVChannelControllerTest` / Mock 테스트가 **스텁 1건**만 존재 | 테스트 부재 | 리팩토링 안전망 없음 | README T-07~T-28 Red → Green 후 refactor | **1** (기능) |

---

## 2. SRP / OCP 위반 지점과 근거

### 2.1 SRP — 한 클래스에 섞인 책임

```7:41:include/TVChannelController.h
class TVChannelController {
    ITuner& tuner_;
    int inputBuffer_ = -1;
    std::vector<int> favorites_;

    bool isValidChannel(int ch) const {
        return ch >= 0 && ch <= 99;
    }

    bool isFavorite(int ch) const { ... }

    void applyChannel(int ch);

public:
    void pressNumber(int digit);
    void pressConfirm();
    void pressFavorite();
    void pressNextFavorite();
    void pressOther();
    // ...
    void addFavorite(int ch) { ... push_back + sort ... }
};
```

| 책임 | 멤버·메서드 | 분리 후보 |
|------|-------------|-----------|
| 숫자 입력 상태 머신 | `inputBuffer_`, `pressNumber`, `pressConfirm`, `pressOther` | `ChannelInputBuffer` (`-1` 센티넬, `*10+digit`, `clear()`) |
| 채널 적용·유효성 | `isValidChannel`, `applyChannel`, `tuner_.setCH` | `applyChannel` 파이프라인 (private 단일 진입점) |
| 선호 채널 CRUD·정렬 | `favorites_`, `isFavorite`, `addFavorite`, `pressFavorite` | `FavoriteChannelList` |
| 다음 선호 탐색 | `pressNextFavorite` (+ `upper_bound`) | `findNextFavorite(current)` |
| 리모컨 표면 | public `press*` 5개 | **Facade** — 유지 적절 |

**근거:** 변경 이유가 서로 다른 세 정책(입력·즐겨찾기·튜너 연동)이 동일 클래스에 쌓인다. README dev/refactoring 브랜치가 `clearBuffer`, `findNextFavorite`, `addToFavorites` 추출을 명시한 것은 SRP 회복을 전제로 한다.

### 2.2 OCP

| 영역 | 평가 | 근거 |
|------|------|------|
| 튜너 구현 교체 | ✅ 유리 | `ITuner&` + `FakeTuner` / `MockTuner` — 컨트롤러 수정 없이 확장 |
| 버튼·입력 정책 확장 | ⚠️ 불리 | `pressNumber` 등 고정 API; 새 버튼마다 헤더·cpp·상태 상호작용 수정 |
| 채널 상한 `0~99` | ⚠️ 불리 | `isValidChannel`·`FakeTuner::setCH`에 분산 — 상한 변경 시 다중 수정 |
| `ITuner.h` 수정 불가 | 의도적 제약 | 계약 확장은 **구현체 교체**로만 — OCP는 DIP 쪽에 치우침 |

```3:16:src/TVChannelController.cpp
void TVChannelController::pressNumber(int digit) { }
void TVChannelController::pressConfirm() { }
// ... 전부 빈 구현
```

구현 완료 후에도 **버튼 종류 확장**은 OCP에 불리하나, 과제 범위(5버튼)에서는 수용 가능하다.

---

## 3. Magic Number 상수화

| 매직 값 | 현재·예정 위치 | 권장 이름 | 필요성 |
|---------|----------------|-----------|--------|
| `-1` | `inputBuffer_` Idle | `kEmptyInput` / `ChannelInputBuffer::empty()` | **높음** — `0`은 유효 채널과 혼동 방지 |
| `0`, `99` | `isValidChannel`, `FakeTuner::setCH` | `kMinChannel`, `kMaxChannel` | **높음** — DRY·테스트 일치 |
| `10` | 두 자리 `inputBuffer_ * 10 + digit` (구현 예정) | `kDecimalBase` 또는 버퍼 캡슐화 | **중간** |
| `0~9` | `pressNumber(digit)` 인자 검증 | `kMinDigit`, `kMaxDigit` | **중간** |
| `100` | 무효 상한 (T-14) | `kMaxChannel + 1` 문서화 | **중간** |

```23:27:include/FakeTuner.h
    void setCH(std::string ch) override {
        int v = std::stoi(ch);
        if (v < 0 || v > 99)
            throw std::invalid_argument("채널 범위 초과: " + ch);
```

```12:14:include/TVChannelController.h
    bool isValidChannel(int ch) const {
        return ch >= 0 && ch <= 99;
    }
```

**권장:** `namespace ChannelLimits { inline constexpr int kMinChannel = 0; inline constexpr int kMaxChannel = 99; }` (C++17)를 공유 헤더로 두고, 컨트롤러·FakeTuner가 동일 상수를 참조한다.

---

## 4. Code Smell 상세

| 스멜 | 해당 코드·설계 | 설명 |
|------|----------------|------|
| **Long Method** | `pressNumber` 구현 예정 | Idle/Buffered 분기 + 두 자리 즉시 적용 + 예외 → `ChannelInputBuffer::onDigit(d)`로 축소 |
| **Duplicated Code** | 버퍼 `-1`, `addFavorite`/`pressFavorite`, 범위 `0~99` | README refactor 항목과 1:1 대응 |
| **조건문 복잡도** | `pressNumber`: `inputBuffer_ == -1` vs 두 자리 | early return·상태 객체로 중첩 감소 |
| **Feature Envy** | `pressFavorite` → `tuner_.getCurrentCH()` + `favorites_` 직접 조작 | `FavoriteChannelList::toggleFromTuner(ITuner&)` 등으로 응집 |
| **Controller·Fake 검증 중복** | `isValidChannel` vs `setCH` | `applyChannel`에서 선검증 후 Fake는 최종 방어 — Mock은 `setCH` 호출만 검증 |
| **Dead Code** | `.cpp` 빈 메서드 | TDD 미진행 상태의 구조적 부채 |

**FakeTuner 알고리즘 일관성:** `seekCH`는 `find_if(> current_)`를, 선호 채널은 `upper_bound`를 사용한다. 의미는 동일(정렬 목록에서 다음 큰 값)하나, 리팩토링 시 **동일 패턴**으로 통일하면 인지 부하가 줄어든다.

---

## 5. C++17 스타일·구조 개선 (README dev/refactoring 정합)

| README refactor | 개선 방향 |
|-----------------|-----------|
| `clearBuffer()` | `ChannelInputBuffer::clear()` 또는 private `clearBuffer()` |
| `findNextFavorite()` | `std::upper_bound` + wrap → `favorites_.front()` |
| `isValidChannel` → `applyChannel` 내부 전용 | 외부에서 채널 검증 분산 금지 |
| `isFavorite` 재사용 | `pressFavorite` / `pressNextFavorite` 공통 |
| `addToFavorites()` | push + sort 한곳 |
| 테스트 픽스처·GWT·lcov 80%+ | 구조 안정 후 (우선순위 5) |

**의도된 `applyChannel` 파이프라인:**

```cpp
void TVChannelController::applyChannel(int ch) {
    if (!isValidChannel(ch))
        throw std::invalid_argument(/* ch */);
    tuner_.setCH(std::to_string(ch));
}
```

**중장기 (과제 범위 초과 시):** `using ButtonHandler = std::function<void(TVChannelController&)>;` + 버튼 ID 맵 — OCP 강화. 현 단계 5개 `press*`는 유지해도 된다.

---

## 6. Test Double 설계와의 관계

| 요소 | 현재 설계 | 품질·테스트 연계 |
|------|-----------|------------------|
| `ITuner&` DI | `explicit TVChannelController(ITuner& t)` | **DIP 유지 필수** — 구체 튜너 비의존 |
| **FakeTuner** | `current_`, `setCH` 범위 검증, `getCurrentCH` | **상태 기반** — T-07~T-23, 최종 채널·`getFavoriteChannels()` |
| **MockTuner** | GMock 3메서드 | **행위 기반** — `setCH` 횟수·`getCurrentCH`→`setCH` 순서 (T-24~T-28) |
| 검증 경계 | Fake = 결과, Mock = 협력 | `applyChannel`이 **유일한 `setCH` 진입점**이면 Mock 기대 단순화 |

**Fake vs Mock 검증 경계**

- Fake: `pressNumber(1,2)` 후 `tuner.getCurrentCH() == "12"` (상태)
- Mock: `EXPECT_CALL(setCH("12")).Times(1)` (협력)
- 즐겨찾기 **목록 내용**은 Fake 테스트; Mock은 `getCurrentCH`/`setCH` **호출 여부·순서**만
- `seekCH`는 컨트롤러 시나리오에서 호출되지 않음 — Mock/Fake 모두 불필요 호출 0 검증 가능

채널 범위 로직이 Controller·Fake에 **이중**이면, 컨트롤러가 검증을 생략하고 Fake에만 맡길 경우 T-14(컨트롤러 예외)와 불일치할 수 있다. **`applyChannel` 선검증**을 표준으로 한다.

---

## 7. 리팩토링 우선순위 (1~5) — Green 테스트 유지 전제

| 순위 | 작업 | 이유 |
|------|------|------|
| **1** | TDD Green: `applyChannel` → `pressNumber` / `pressConfirm` / `pressOther` / `pressFavorite` / `pressNextFavorite` | 스텁 제거 없이는 구조 리팩토링 근거 없음; README 28 시나리오가 안전망 |
| **2** | `clearBuffer()`, `applyChannel` 파이프라인, `isValidChannel` 캡슐화, `ChannelLimits` | README refactor 직결; 버퍼·범위 버그 예방 (T-10, T-14) |
| **3** | `findNextFavorite()`, `addToFavorites()` / toggle 통합 | `upper_bound` 복잡도 격리; `addFavorite` vs `pressFavorite` 중복 제거 |
| **4** | `ChannelInputBuffer` / `FavoriteChannelList` 클래스 추출, `binary_search`, API 정리 | SRP 회복; dev/refactoring 브랜치 목표 |
| **5** | 테스트 픽스처·Given-When-Then·lcov 80%+·`FakeTuner` 빈 `available_` 방어 | 품질 게이트·엣지 방어 |

---

## 8. 개선 방향 요약

1. **단기 (dev / Green):** `applyChannel`을 채널 변경·예외의 **유일 관문**으로 구현하고, 모든 `press*`가 이 경로만 사용하도록 한다. README T-01~T-28을 Red → Green으로 채운다.
2. **중기 (dev/refactoring):** 입력 버퍼(`clearBuffer` / `ChannelInputBuffer`), 선호 목록(`addToFavorites`, `findNextFavorite`), `applyChannel` 파이프라인을 README refactor TODO와 **1:1**로 추출해 SRP를 회복한다.
3. **상수·정책:** `-1`, `0`, `99`, `10`, digit `0~9`를 named constant 또는 `ChannelLimits`로 모아 **FakeTuner**와 **컨트롤러**가 동일 규칙을 공유한다.
4. **Test Double:** `ITuner&` DI는 유지한다. Fake는 관찰 가능한 상태, Mock은 `applyChannel` 경유 `setCH`/`getCurrentCH` **협력 계약**만 검증한다.
5. **OCP:** 과제 범위(5버튼)에서는 `press*` API로 충분하다. 기능 확장 시 Command/Handler 레지스트리를 검토한다.

**종합:** 헤더·`ITuner` 추상화는 **DIP·테스트 가능성** 측면에서 양호하다. `FakeTuner`는 동작 가능하나, `TVChannelController`는 **다중 책임이 헤더에 이미 응집**되어 있고 `.cpp`는 스텁이라 SRP·DRY 이슈는 Green 직후 리팩토링(우선순위 2~3)으로 해소하는 것이 README dev/refactoring과 정합된다.
