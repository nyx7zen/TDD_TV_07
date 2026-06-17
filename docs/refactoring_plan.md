# TVChannelController 리팩토링 계획 (Step 7)

| 항목 | 내용 |
|------|------|
| 대상 | `TVChannelController`, `FakeTuner` (연관 테스트·Golden Master) |
| 브랜치 | `dev/refactoring` (모든 단계에서 테스트 Green 유지) |
| 기준 문서 | `README.md` dev/refactoring, `docs/code_quality_report.md`, `docs/requirements_analysis.md` |
| 작성일 | 2026-05-19 |
| 사전 조건 | 단위 테스트 + Golden Master **전부 Green** (기준선 확인 완료) |

---

## 제약 (변경 금지·불변)

| 항목 | 내용 |
|------|------|
| `ITuner.h` | **수정 금지** — `setCH` / `getCurrentCH` / `seekCH` 계약 유지 |
| 채널 범위 | **0~99** 유지 (`isValidChannel`, `FakeTuner::setCH`, T-14/T-03/T-04) |
| 공개 API | `pressNumber` / `pressConfirm` / `pressFavorite` / `pressNextFavorite` / `pressOther` 시그니처 유지 |
| 동작 | 리팩토링은 **구조·가독성만** 변경; 요구사항·Golden approved 파일은 의도적 변경 전까지 동일 |
| 검증 명령 | 각 커밋 후 아래 **표준 검증** 실행 |

### 표준 검증 (모든 단계 공통)

```powershell
cd c:\DEV\week2_day2\TDD_TV_07
cmake --build build
ctest --test-dir build --output-on-failure
```

| 기대 결과 | 설명 |
|-----------|------|
| 4/4 Pass | `fake_tuner_test`, `controller_test`, `controller_mock_test`, `golden_test` |
| 선택 | `lcov` / HTML 리포트로 커버리지 80%+ (README 최종 게이트) |

---

## 현재 코드 스냅샷 (리팩토링 출발점)

### 중복·복잡도 핫스팟

| 위치 | 이슈 | README 매핑 |
|------|------|-------------|
| `inputBuffer_ = -1` | `pressOther`, `pressConfirm`, `pressNumber` 다수 | `clearBuffer()` |
| `pressNextFavorite` | `upper_bound` + wrap 인라인 | `findNextFavorite()` |
| `pressFavorite` vs `addFavorite` | push+sort / erase+remove 이중 경로 | `addToFavorites()` |
| `isValidChannel` vs `FakeTuner::setCH` | `0`/`99` 리터럴 이중 | `ChannelLimits` |
| `pressNumber` | Idle / 한 자리 / `next==10` 보류 / 세 자리 분기 | `ChannelInputBuffer` (후기) |
| `applyChannel` | 이미 단일 `setCH` 관문 — **유지·강화** | `applyChannel` 파이프라인 |

### 보존해야 할 입력 상태 머신 (회귀 주의)

`pressNumber`에서 `next == 10`일 때 **즉시 적용하지 않고** `inputBuffer_ = 10`으로 보류하는 분기가 있다. 채널 10은 `1` → `0` 후 `pressConfirm()` 또는 추가 숫자 입력으로 확정된다. 이 동작은 T-07~T-14 및 Golden `channel_*` 시나리오에 묶여 있으므로, `ChannelInputBuffer` 추출 시 **시나리오 테스트로 재확인**한다.

---

## 단계별 계획 (커밋 단위)

각 단계는 **한 커밋 = 한 논리 변경**. 실패 시 해당 커밋만 revert.

### Phase 0 — 기준선 고정

| # | 작업 | 체크리스트 |
|---|------|------------|
| 0.1 | `dev/refactoring` 브랜치 생성·체크아웃 | [ ] `git checkout -b dev/refactoring` (또는 기존 브랜치 사용) |
| 0.2 | Green 기록 | [ ] `ctest` 4/4 Pass 스크린샷 또는 CI 로그 보관 |
| 0.3 | Golden 기준선 | [ ] `golden_test` Pass, `test/golden/*.approved.txt` 변경 없음 확인 |

**검증:** 표준 검증만.

---

### Phase 1 — 매직 넘버 상수화 (`ChannelLimits`)

**목표:** 컨트롤러·FakeTuner가 동일 채널·숫자 규칙을 공유. **동작 변경 없음.**

| # | 작업 | 체크리스트 |
|---|------|------------|
| 1.1 | `include/ChannelLimits.h` 추가 | [ ] `kMinChannel=0`, `kMaxChannel=99`, `kEmptyInput=-1`, `kDecimalBase=10`, `kMinDigit=0`, `kMaxDigit=9` |
| 1.2 | `TVChannelController::isValidChannel` | [ ] `ChannelLimits` 참조로 교체 |
| 1.3 | `FakeTuner::setCH` | [ ] 동일 상수로 범위 검사 (방어적 검증 유지) |
| 1.4 | `pressNumber` | [ ] `99`, `10`, `-1` 리터럴 → 상수 (로직 동일) |

**검증:** 표준 검증 + `FakeTunerTest` 경계(`0`, `99`, `-1`, `100`) Pass.

**커밋 메시지 예:** `refactor: extract ChannelLimits shared constants`

**Fake vs Mock:** Fake 경계 테스트만 영향 가능; Mock은 `setCH` 문자열 인자 불변 → **회귀 낮음**.

---

### Phase 2 — `clearBuffer()` 추출

**목표:** 버퍼 초기화 단일화 (T-10 버퍼 무효화 버그 예방).

| # | 작업 | 체크리스트 |
|---|------|------------|
| 2.1 | `TVChannelController` private `void clearBuffer()` | [ ] `inputBuffer_ = ChannelLimits::kEmptyInput` |
| 2.2 | 치환 | [ ] `pressOther()` |
| 2.3 | 치환 | [ ] `pressConfirm()` 적용 후 |
| 2.4 | 치환 | [ ] `pressNumber()` 두 자리·세 자리 완료 후 모든 `-1` 대입 |

**검증:** 표준 검증; 특히 `버퍼입력후Other_45유지_6미적용`, Golden `channel_buffer_cleared_by_other`.

**커밋 메시지 예:** `refactor: extract clearBuffer for input sentinel reset`

---

### Phase 3 — `applyChannel` 파이프라인 정리

**목표:** 채널 변경·예외의 **유일 관문** 명시; `isValidChannel`은 `applyChannel` 전용.

| # | 작업 | 체크리스트 |
|---|------|------------|
| 3.1 | `isValidChannel` | [ ] private 유지, 다른 public 경로에서 직접 호출 없음 |
| 3.2 | `applyChannel` | [ ] 검증 → `tuner_.setCH(std::to_string(ch))` 순서 고정 |
| 3.3 | 모든 채널 변경 | [ ] `pressNumber` / `pressConfirm` / `pressNextFavorite`가 `applyChannel`만 경유 |
| 3.4 | 예외 메시지 | [ ] 기존 `"채널 범위 초과: "` 형식 유지 (Golden·T-14) |

**검증:** 표준 검증; Mock `setCH` 기대 문자열 불변; Golden `exception_channel_100`.

**Fake vs Mock:**

| 테스트 종류 | 확인 포인트 |
|-------------|-------------|
| Fake (`controller_test`) | 최종 `getCurrentCH()` 상태 |
| Mock (`controller_mock_test`) | `setCH("…")` **횟수·인자** 동일 |
| Golden | approved 트레이스 바이트 동일 |

---

### Phase 4 — `findNextFavorite()` 추출

**목표:** `upper_bound` + wrap-around 로직 격리 (T-19~T-23).

| # | 작업 | 체크리스트 |
|---|------|------------|
| 4.1 | `int findNextFavorite(int current) const` | [ ] `std::upper_bound(favorites_.begin(), favorites_.end(), current)` |
| 4.2 | wrap | [ ] `it == end` → `favorites_.front()` |
| 4.3 | `pressNextFavorite` | [ ] 빈 목록 early return 유지 → `findNextFavorite` → `applyChannel` |

**검증:** `다음선호_*`, Golden `next_favorite_*` 시나리오.

**커밋 메시지 예:** `refactor: extract findNextFavorite with upper_bound wrap`

---

### Phase 5 — 선호 채널 CRUD: `addToFavorites` / `removeFromFavorites`

**목표:** push+sort·erase 중복 제거; 정렬 불변식 한곳에서 유지.

| # | 작업 | 체크리스트 |
|---|------|------------|
| 5.1 | `void addToFavorites(int ch)` | [ ] 중복 시 no-op, `push_back` + `std::sort` |
| 5.2 | `void removeFromFavorites(int ch)` | [ ] `erase` + `remove` 패턴 캡슐화 |
| 5.3 | `pressFavorite` | [ ] `getCurrentCH` → 토글 시 `add`/`remove` 위임 |
| 5.4 | `addFavorite` (public, 테스트 Arrange) | [ ] 내부적으로 `addToFavorites` 호출 (동작 동일) |
| 5.5 | `isFavorite` | [ ] `pressFavorite` / 필요 시 `addToFavorites`에서 재사용 |

**검증:** T-15~T-18, Mock `다음선호_목록있음` (`addFavorite` Arrange), Golden `favorites_complex_toggle`.

**Fake vs Mock:** Mock은 **목록 내용**을 검증하지 않음 → `addFavorite` 구현 위임만으로 Mock Pass 유지.

**후속 (선택, 별 커밋):** `std::binary_search`로 `isFavorite` 최적화 — 정렬 전제 문서화 후.

---

### Phase 6 — `pressNumber` 조건 분기 축소

**목표:** 분기 깊이 감소; Phase 2·3에서 만든 `clearBuffer`/`applyChannel` 활용.

| # | 작업 | 체크리스트 |
|---|------|------------|
| 6.1 | private 헬퍼 (이름 예시) | [ ] `void bufferFirstDigit(int digit)` — Idle → Buffered |
| 6.2 | | [ ] `void bufferSecondDigit(int digit)` — `*10+digit`, `next==10` 보류, 즉시 적용, 예외 |
| 6.3 | | [ ] `void bufferThirdDigit(int digit)` — `inputBuffer_==10` 경로 |
| 6.4 | `pressNumber` | [ ] 위 헬퍼로 early-return 구조; **분기 결과 동일** |
| 6.5 | (선택) digit 검증 | [ ] `0~9` 범위 — 요구사항에 명시 시만 추가 (기존 테스트 Green 유지) |

**검증:** T-07~T-14 전부; Golden `channel_leading_zero_07`, `channel_two_digit_immediate`, `channel_sequence_12_then_34`, `channel_99_boundary`.

**커밋:** 헬퍼 추출 1커밋, `pressNumber` 본문 정리 1커밋으로 분리 가능.

---

### Phase 7 — 타입·책임 분리 (SRP)

**목표:** README 중기 목표 — 컨트롤러는 Facade, 정책은 협력 클래스.

#### 7A — `ChannelInputBuffer`

| # | 작업 | 체크리스트 |
|---|------|------------|
| 7A.1 | `include/ChannelInputBuffer.h` | [ ] 센티넬 `kEmpty`, `clear()`, `onDigit(int, applyFn)` 또는 동등 API |
| 7A.2 | `TVChannelController` | [ ] `int inputBuffer_` → `ChannelInputBuffer buffer_` |
| 7A.3 | | [ ] `pressNumber`/`pressConfirm`/`pressOther`가 버퍼에 위임 |

#### 7B — `FavoriteChannelList`

| # | 작업 | 체크리스트 |
|---|------|------------|
| 7B.1 | `include/FavoriteChannelList.h` | [ ] `add`, `remove`, `contains`, `findNext(int current)`, `asVector()` |
| 7B.2 | `TVChannelController` | [ ] `std::vector<int> favorites_` → `FavoriteChannelList favorites_` |
| 7B.3 | `getFavoriteChannels()` | [ ] 기존 const 참조 계약 유지 |

**검증:** 표준 검증 + Golden 전 시나리오.

**커밋:** 7A / 7B **각각 별 커밋** 권장.

---

### Phase 8 — (선택) Command / Handler · OCP

과제 범위(버튼 5개)에서는 **필수 아님**. Green·lcov 달성 후 검토.

| # | 작업 | 체크리스트 |
|---|------|------------|
| 8.1 | `enum class ButtonId` | [ ] Number, Confirm, Favorite, NextFavorite, Other |
| 8.2 | | [ ] `std::unordered_map<ButtonId, std::function<void()>>` 또는 멤버 함수 포인터 테이블 |
| 8.3 | public `press*` | [ ] 기존 메서드는 맵/핸들러에 위임만 |

**주의:** Mock 테스트는 `pressNumber` 등 **기존 public API**를 호출 — 시그니처 유지 필수.

---

### Phase 9 — 테스트 코드 리팩토링 (README dev/refactoring)

프로덕션 Green 후 진행. **동작 검증 테스트는 수정 최소화.**

| # | 작업 | 체크리스트 |
|---|------|------------|
| 9.1 | `ControllerTest::SetUp` / `tuneTo` | [ ] 중복 채널 설정 통합 (이미 `tuneTo` 있으면 주석·네이밍 정리) |
| 9.2 | Given-When-Then | [ ] 누락 시나리오 보완 |
| 9.3 | 테스트명 | [ ] `동작_조건_기대결과` 패턴 통일 |
| 9.4 | ASSERT vs EXPECT | [ ] Arrange=ASSERT, Then=EXPECT 기준 문서화 |

**검증:** 표준 검증 (테스트 리네임만으로 CTest 이름 변경 시 Windows 등록 방식 확인).

---

### Phase 10 — 커버리지·최종 게이트

| # | 작업 | 체크리스트 |
|---|------|------------|
| 10.1 | lcov | [ ] `TVChannelController.cpp` 미커버 라인 식별 |
| 10.2 | 보완 테스트 | [ ] 실제 미커버 분기만 추가 (Golden·단위 중복 최소화) |
| 10.3 | README | [ ] dev/refactoring 체크리스트 항목 전부 [x] |
| 10.4 | push | [ ] `dev/refactoring` → remote |

---

## Fake vs Mock 회귀 영향 매트릭스

리팩토링 단계마다 **어느 테스트가 깨지기 쉬운지** 미리 점검한다.

| 변경 | Fake (`controller_test`) | Mock (`controller_mock_test`) | Golden |
|------|--------------------------|-------------------------------|--------|
| `ChannelLimits` | 경계 setCH/예외 | 영향 없음 | 영향 없음 |
| `clearBuffer` | T-10 버퍼 무효화 | 영향 없음 | `channel_buffer_cleared_by_other` |
| `applyChannel` 메시지/경로 | T-14 예외·채널 상태 | **setCH 인자·횟수** | exception·channel 시나리오 |
| `findNextFavorite` | T-19~T-23 | `getCurrentCH`→`setCH` 순서 | `next_favorite_*` |
| `addToFavorites` | 목록 정렬·토글 | `addFavorite` Arrange만 | `favorites_*` |
| `ChannelInputBuffer` | **전체 숫자 입력** | 두 자리 `setCH("12")` 등 | **대부분 channel_*** |
| `FavoriteChannelList` | 목록 내용 | Mock은 목록 미검증 | favorites 시나리오 |
| Command 맵 | 동일 public API면 없음 | 동일 | 동일 |

**원칙**

- **Fake:** 관찰 가능한 **최종 상태** (`getCurrentCH`, `getFavoriteChannels`).
- **Mock:** **협력 계약** (`setCH`/`getCurrentCH` 호출 순서·횟수); 내부 `favorites_` 구조 변경은 Mock에 무관.
- **Golden:** E2E 트레이스; `pressNumber` 상태 머신 변경 시 **최우선** 회귀 기준.

---

## README dev/refactoring TODO 매핑

| README 항목 | 본 계획 Phase |
|-------------|----------------|
| `clearBuffer()` | Phase 2 |
| `findNextFavorite()` | Phase 4 |
| `isValidChannel` → `applyChannel` 전용 | Phase 3 |
| `isFavorite` 재사용 | Phase 5 |
| `applyChannel` 파이프라인 | Phase 3 |
| `addToFavorites()` | Phase 5 |
| `addFavorite` / 내부 로직 중복 제거 | Phase 5 |
| `ChannelInputBuffer` / `FavoriteChannelList` | Phase 7 |
| 테스트 픽스처·GWT·이름 | Phase 9 |
| lcov 80%+ | Phase 10 |

---

## 권장 커밋 순서 (요약)

```
Phase 0  기준선
Phase 1  ChannelLimits
Phase 2  clearBuffer
Phase 3  applyChannel 정리
Phase 4  findNextFavorite
Phase 5  addToFavorites / removeFromFavorites
Phase 6  pressNumber 분기 축소
Phase 7A ChannelInputBuffer
Phase 7B FavoriteChannelList
Phase 8  (선택) Command/Handler
Phase 9  테스트 가독성
Phase 10 lcov + README 체크리스트
```

---

## 리스크·롤백

| 리스크 | 완화 |
|--------|------|
| `next==10` 보류 로직 훼손 | Phase 6·7 전후 Golden `channel_*` 필수 실행 |
| Mock `setCH` 문자열 불일치 | `applyChannel`에서만 `to_string`; 포맷 변경 금지 |
| `addFavorite` 제거 시 테스트 깨짐 | public API 유지, 구현만 `addToFavorites` 위임 |
| 클래스 추출 시 헤더 순환 | `ChannelInputBuffer`는 `ITuner` 비의존, `applyChannel`은 콜백/함수 인자로 주입 |

**롤백:** 단계별 커밋 → `git revert <commit>` → 표준 검증.

---

## 완료 정의 (Definition of Done)

- [ ] `ITuner.h` 미수정
- [ ] 채널 0~99, 예외·Golden·T-14 동작 동일
- [ ] `ctest` 4/4 Pass (단위 + Golden)
- [ ] README dev/refactoring 체크리스트 전항목 완료
- [ ] (목표) lcov **80%+** on `TVChannelController`
- [ ] `docs/refactoring_plan.md`와 실제 커밋 이력 대응

---

## 부록: 목표 구조 (Phase 7 완료 후)

```
TVChannelController (Facade)
├── ITuner& tuner_
├── ChannelInputBuffer buffer_     // clear, onDigit → applyChannel
├── FavoriteChannelList favorites_ // add/remove/findNext, sorted invariant
└── applyChannel(int)              // sole setCH + validation gateway
```

`applyChannel`만 `ITuner::setCH`를 호출한다는 불변식을 유지하면, Mock 테스트는 리팩토링 내내 **안정적인 회귀 기준**이 된다.
