# TDD TV Channel Controller — 테스트 계획서

| 항목 | 내용 |
|------|------|
| 단계 | Step 3 — 테스트 계획 |
| 역할 | 시니어 QA 리드 |
| 프로젝트 | TDD TV Channel Controller (C++17, CMake, Google Test / GMock) |
| 기준 문서 | `README.md` TO-DO 28건, `Report/step-1.requirements_analysis.md` (T-01~T-28) |
| SUT | `FakeTuner`, `TVChannelController` (`src/TVChannelController.cpp` — 현재 스텁) |
| 커버리지 목표 | README **≥ 80%** 라인 · 권장 **≥ 90%** |
| 측정 | gcov + lcov (가능 시 CMake 옵션) |
| 작성일 | 2026-05-19 |
| 동기 문서 | `docs/test_plan.md` |

---

## 1. 단위 테스트 범위·우선순위

### 1.1 CMake 실행 파일·검증 범위

| 순서 | CMake 타겟 | 소스 | SUT / 범위 | Test Double | 검증 스타일 |
|------|------------|------|------------|-------------|-------------|
| **1** | `fake_tuner_test` | `test/FakeTunerTest.cpp` | `FakeTuner` 단독 | — | **상태** (`getCurrentCH`, 예외) |
| **2** | `controller_test` | `test/TVChannelControllerTest.cpp` | `TVChannelController` + 튜너 통합 | `FakeTuner` | **상태** (최종 채널·`favorites_`) |
| **3** | `controller_mock_test` | `test/TVChannelControllerMockTest.cpp` | 컨트롤러 ↔ `ITuner` 협력 | `MockTuner` | **행위** (`EXPECT_CALL`, `InSequence`) |

현재 `CMakeLists.txt`는 위 3타겟·`gtest_discover_tests` 등록이 완료되어 있다. 커버리지 플래그는 **Phase F**(§5)에서 옵션으로 추가한다.

### 1.2 TDD Red → Green 진행 순서

과제는 **테스트 선행(Red) → 최소 구현(Green)** 을 파일·기능 단위로 진행한다. Mock 테스트는 컨트롤러가 Fake 테스트로 기능이 맞춰진 뒤 **협력 계약**을 고정하는 용도로 둔다.

```text
Phase A  fake_tuner_test     T-01 ~ T-06   FakeTuner 경계·seekCH wrap
Phase B  controller_test     T-07 ~ T-14   기능 1: 숫자·확정·Other·예외
Phase C  controller_test     T-15 ~ T-18   기능 2: 선호 토글·정렬
Phase D  controller_test     T-19 ~ T-23   기능 3: next favorite·wrap
Phase E  controller_mock_test T-24 ~ T-28  setCH 횟수·순서·미호출
Phase F  (리팩토링 브랜치)   lcov ≥ 80%    미커버 분기 보완 → 90% 권장
```

**우선순위 요약**

| 등급 | 범위 | 이유 |
|------|------|------|
| **P0** | T-01~T-04, T-07~T-08, T-14 | 튜너·컨트롤러 **계약 핵심**(경계·두 자리·예외) |
| **P1** | T-05~T-06, T-09~T-13, T-15~T-21, T-24~T-26, T-28 | README 명세 전체·Mock 기본 5건 |
| **P2** | T-17, T-22~T-23, T-27 | 복합 시퀀스·목록 외·호출 순서 |

### 1.3 `TEST` vs `TEST_F` 사용 기준

| 파일 | 패턴 | 적용 기준 |
|------|------|-----------|
| `FakeTunerTest.cpp` | **`TEST` 위주** | 픽스처 공유 불필요; `FakeTuner({1,4,12,56})` 로컬 생성 |
| `TVChannelControllerTest.cpp` | **`TEST_F(ControllerTest, ...)`** | `FakeTuner` + `TVChannelController` + 초기 `setCH("0")` 반복 제거 |
| `TVChannelControllerMockTest.cpp` | **`TEST_F(MockControllerTest, ...)`** | `ON_CALL` 기본값·`StrictMock`·`InSequence` 공통 |

**픽스처 예시 (리팩토링 TODO: SetUp 통합)**

```cpp
class ControllerTest : public ::testing::Test {
protected:
    FakeTuner tuner{std::vector<int>{1, 4, 12, 56}};
    TVChannelController ctrl{tuner};

    void SetUp() override { tuner.setCH("0"); }
};
```

**`ASSERT_*` vs `EXPECT_*`**

| 매크로 | 용도 |
|--------|------|
| `ASSERT_*` | 이후 Act/Assert가 무의미해지는 **전제**(Arrange 실패) |
| `EXPECT_*` | **결과 검증**(채널 문자열, favorites, `EXPECT_THROW`) |

### 1.4 파일별 테스트 건수·현재 상태

| 파일 | 계획 | 현재 |
|------|------|------|
| `FakeTunerTest.cpp` | 6 (`TEST`) | 1 스텁 (`Stub`) |
| `TVChannelControllerTest.cpp` | 17 (`TEST_F`) | 1 스텁 |
| `TVChannelControllerMockTest.cpp` | 5 (`TEST_F`) | 1 스텁 |
| **합계** | **28** | **3** — T-01~T-28 **미착수** |

---

## 2. 경계값 케이스 목록

### 2.1 채널 정수·문자열 (0~99)

| ID | 값 | 경계 의미 | 검증 위치 | 시나리오 |
|----|-----|-----------|-----------|----------|
| B-CH-00 | **0** | 최솟값 | Fake `setCH` / Controller | T-01, T-12 |
| B-CH-01 | **1** | 최소 양수·한 자리 | Controller | T-07 (`1`+Confirm → `"1"`) |
| B-CH-98 | **98** | 상한 직전 (확장) | Controller | `9`,`8` — **권장 보완** |
| B-CH-99 | **99** | 최댓값 | Fake / Controller | T-02, T-13 (`9`,`9`) |
| B-CH--1 | **-1** | 무효 하한 | Fake `setCH("-1")` | T-03 → `invalid_argument` |
| B-CH-100 | **100** | 무효 상한 | Fake / Controller | T-04, T-14 (`1`,`0`,`0`) |

**인접 경계 조합 (기능 1)**

| 입력 시퀀스 | 산술값 | 기대 |
|-------------|--------|------|
| `0` + Confirm | 0 | `"0"` (T-12) |
| `0`, `7` | 7 | `"7"` (T-11) |
| `9`, `9` | 99 | `"99"` (T-13) |
| `9`, `8` | 98 | `"98"` (B-CH-98) |
| `1`, `0`, `0` | 100 | 예외 (T-14) |

### 2.2 숫자 버튼 `pressNumber(digit)` — digit 0~9

| digit | 역할 예시 | 관련 시나리오 |
|-------|-----------|---------------|
| 0 | 선행 0·채널 0 | T-11, T-12 |
| 1~8 | 일반 두 자리 조합 | T-07~T-10, T-19 |
| 9 | `99` 상한 구성 | T-13 |

### 2.3 `inputBuffer_` 상태

| 값 | 의미 | 검증 |
|----|------|------|
| **-1** | Idle | `pressConfirm` on Idle → 무동작 (F1-10) |
| **0~9** | 한 자리 대기 | 두 번째 `pressNumber` / `pressConfirm` / `pressOther` |

| 전이 | Act | `inputBuffer_` after | 튜너 |
|------|-----|----------------------|------|
| Idle → Buffered | `pressNumber(4)` | `4` | 변화 없음 |
| Buffered → Idle (적용) | `pressNumber(5)` | `-1` | `"45"` 즉시 |
| Buffered → Idle (무효화) | `pressOther()` | `-1` | 이전 채널 유지 (T-10) |
| Buffered → Idle (확정) | `pressConfirm()` | `-1` | 한 자리 적용 (T-07) |

### 2.4 `favorites_` 크기·wrap-around

| ID | `favorites_` | 현재 채널 | 기대 | 시나리오 |
|----|--------------|-----------|------|----------|
| B-FAV-0 | `{}` | 임의 | 채널 변화 없음 | T-21, T-28 |
| B-FAV-1 | `{12}` | `12` | wrap → `"12"` | T-23 |
| B-FAV-N | `{1,4,12,56}` | `6` | next → `"12"` | T-19 |
| B-FAV-N | 동일 | `56` | wrap → `"1"` | T-20 |
| B-FAV-외 | `{1,4,12,56}` | `6` (∉ 목록) | `upper_bound` → `12` | T-22 |

### 2.5 `FakeTuner::seekCH` wrap-around

| 단계 | `available_` | `current_` | `seekCH()` 결과 |
|------|--------------|------------|-----------------|
| 초기 | `{1,4,12,56}` | 0 | `"1"` |
| 연속 | 동일 | 마지막 이후 | `available_.front()` (T-06) |
| 유효성 | — | — | 반환값 ∈ [0,99] (T-05) |

---

## 3. 예외·특이 케이스 목록

### 3.1 예외 (`std::invalid_argument`)

| ID | 트리거 | 발생 경로 | 테스트 |
|----|--------|-----------|--------|
| E-01 | `setCH("-1")` | `FakeTuner::setCH` | T-03 |
| E-02 | `setCH("100")` | `FakeTuner::setCH` | T-04 |
| E-03 | `1,0,0` → 100 | `applyChannel` → `tuner_.setCH` | T-14 |
| E-04 | `isValidChannel` false | 컨트롤러 | T-14 |

### 3.2 버퍼·입력 특이

| ID | 시나리오 | 기대 | 테스트 |
|----|----------|------|--------|
| E-05 | `4`,`5`,`6` + `pressOther()` | `"45"` 유지 | T-10 |
| E-06 | 즉시 vs 확정 | `12` / `1`+Confirm | T-08, T-07 |
| E-07 | Confirm on Idle | 튜너 무변경 | F1-10 (P2) |
| E-08 | `1,2,3,4` | `"12"` → `"34"` | T-09 |

### 3.3 선호 채널·다음 채널 특이

| ID | 시나리오 | 기대 | 테스트 |
|----|----------|------|--------|
| E-09 | 미등록 Favorite | 목록 추가 | T-15 |
| E-10 | 재 Favorite | 목록 삭제 | T-16 |
| E-11 | 복합 토글 시퀀스 | `{6,12,37}` | T-17 |
| E-12 | 현재 ∉ favorites | upper_bound / wrap | T-22 |
| E-13 | `getFavoriteChannels()` | 오름차순 | T-18 |

### 3.4 Mock — 미호출·순서

| ID | 시나리오 | GMock 기대 | 테스트 |
|----|----------|------------|--------|
| E-M01 | 빈 favorites + next | `setCH` 0회 | T-28 |
| E-M02 | next fav (목록 있음) | `getCurrentCH` → `setCH` | T-27 `InSequence` |
| E-M03 | Favorite | `getCurrentCH` ≥1 | T-26 |
| E-M04 | Confirm | `setCH("1")` ×1 | T-24 |
| E-M05 | 두 자리 | `setCH("12")` ×1 | T-25 |
| E-M06 | 전반 | `seekCH` 미기대 | — |

---

## 4. Fake vs Mock 검증 분리 전략

### 4.1 역할 분담

| 구분 | Fake (`controller_test`) | Mock (`controller_mock_test`) |
|------|--------------------------|-------------------------------|
| 검증 대상 | 최종 **상태** | **협력 행위** |
| Assert | `tuner.getCurrentCH()`, `getFavoriteChannels()` | `EXPECT_CALL`, `Times`, `InSequence` |
| 예외 | `EXPECT_THROW` | Fake 쪽 위주 |
| favorites | 전체 비교 | 검증 안 함 |
| `seekCH` | `FakeTunerTest` 전용 | 미기대 |

**중복 금지:** 동일 시나리오를 Fake·Mock에 전부 복제하지 않는다. Fake=결과, Mock=호출 계약.

### 4.2 GMock 패턴 요약

- **T-24:** `EXPECT_CALL(mock, setCH("1")).Times(1);`
- **T-27:** `InSequence` + `getCurrentCH` → `setCH`
- **T-28:** `EXPECT_CALL(mock, setCH(_)).Times(0);`
- `ON_CALL`로 기본 반환, 검증 대상만 `EXPECT_CALL`

---

## 5. 커버리지 목표·gcov/lcov 전략

| 지표 | README | 권장 |
|------|--------|------|
| 라인 | **≥ 80%** | **≥ 90%** |

**측정 (GCC/MinGW):** `ENABLE_COVERAGE` CMake 옵션 → `ctest` → `lcov` → `genhtml`.

**미커버 보완:** Confirm+Idle (E-07), Favorite erase (T-16), wrap (T-20, T-23), invalid (T-14), 채널 98 (B-CH-98).

---

## 6. README TO-DO 28건 추적 매트릭스

### 6.1 FakeTunerTest.cpp (6건)

| ID | P | 권장 테스트명 | README 요약 |
|----|---|---------------|-------------|
| T-01 | P0 | `SetCH_MinBoundary_ReturnsZero` | `setCH("0")` |
| T-02 | P0 | `SetCH_MaxBoundary_Returns99` | `setCH("99")` |
| T-03 | P0 | `SetCH_Negative_ThrowsInvalidArgument` | `setCH("-1")` |
| T-04 | P0 | `SetCH_100_ThrowsInvalidArgument` | `setCH("100")` |
| T-05 | P1 | `SeekCH_Repeated_AlwaysValidChannel` | seek 연속 |
| T-06 | P1 | `SeekCH_AtEnd_WrapsToFirst` | seek wrap |

### 6.2 TVChannelControllerTest.cpp — 기능 1 (8건)

| ID | P | 권장 테스트명 | README 요약 |
|----|---|---------------|-------------|
| T-07 | P0 | `PressNumber_ThenConfirm_SetsSingleDigit` | `1`+Confirm |
| T-08 | P0 | `PressNumber_Twice_AppliesTwoDigits` | `1`,`2`→`12` |
| T-09 | P1 | `PressNumber_FourDigits_Applies12Then34` | `1,2,3,4` |
| T-10 | P1 | `PressOther_ClearsBuffer_KeepsChannel` | `4,5,6`+Other |
| T-11 | P1 | `PressNumber_LeadingZero_AppliesSeven` | `0`,`7` |
| T-12 | P1 | `PressNumber_ChannelZero_Applies` | 채널 0 |
| T-13 | P1 | `PressNumber_Channel99_Applies` | 채널 99 |
| T-14 | P0 | `PressNumber_Channel100_Throws` | 100+ 예외 |

### 6.3 TVChannelControllerTest.cpp — 기능 2·3 (9건)

| ID | P | 권장 테스트명 | README 요약 |
|----|---|---------------|-------------|
| T-15 | P1 | `PressFavorite_NotInList_Adds` | 미등록 추가 |
| T-16 | P1 | `PressFavorite_InList_Removes` | 토글 삭제 |
| T-17 | P2 | `PressFavorite_Sequence_FinalList637` | `{6,12,37}` |
| T-18 | P1 | `GetFavoriteChannels_AlwaysSorted` | 정렬 |
| T-19 | P1 | `PressNextFavorite_From6_GoesTo12` | 6→12 |
| T-20 | P1 | `PressNextFavorite_From56_WrapsTo1` | wrap |
| T-21 | P1 | `PressNextFavorite_EmptyList_NoChange` | 빈 목록 |
| T-22 | P2 | `PressNextFavorite_CurrentNotInList_GoesNext` | 목록 외 |
| T-23 | P2 | `PressNextFavorite_SingleItem_Wraps` | 1개 wrap |

### 6.4 TVChannelControllerMockTest.cpp (5건)

| ID | P | 권장 테스트명 | 핵심 Expectation |
|----|---|---------------|------------------|
| T-24 | P1 | `Confirm_CallsSetCHOnce` | `setCH("1")` ×1 |
| T-25 | P1 | `TwoDigits_CallsSetCH12Once` | `setCH("12")` ×1 |
| T-26 | P1 | `PressFavorite_CallsGetCurrentCH` | `getCurrentCH` |
| T-27 | P2 | `PressNextFavorite_CallsGetThenSet_InOrder` | `InSequence` |
| T-28 | P1 | `PressNextFavorite_Empty_NoSetCH` | `setCH` ×0 |

### 6.5 요구사항 ↔ 테스트 교차 참조

| 요구사항 | 테스트 ID |
|----------|-----------|
| F1-01 ~ F1-10 | T-07 ~ T-14, E-07 |
| F2-01 ~ F2-04 | T-15 ~ T-18 |
| F3-01 ~ F3-06 | T-19 ~ T-23, T-28 |
| FT-01 ~ FT-06 | T-01 ~ T-06 |
| MK-01 ~ MK-06 | T-24 ~ T-28 |

---

## 7. 실행·완료 기준

```bash
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure
```

**Definition of Done**

- [ ] T-01 ~ T-28 Green
- [ ] Fake: 상태·예외·favorites / Mock: 협력 5건
- [ ] lcov ≥ 80% (권장 90%)
- [ ] 테스트명·AAA 주석 (README refactoring)

---

## 8. 현재 코드베이스 스냅샷

| 구성요소 | 상태 |
|----------|------|
| `FakeTuner.h` | 구현 완료 |
| `MockTuner.h` | `MOCK_METHOD` 3개 |
| `TVChannelController.cpp` | **스텁** |
| 테스트 | 파일당 Stub 1건 — Phase A(P0) Red 착수 대기 |

본 보고서는 `Report/step-1.requirements_analysis.md` 및 `docs/test_plan.md`와 동기화한다. TDD는 **Phase A → E** 순, 커버리지는 **Phase F**에서 달성한다.
