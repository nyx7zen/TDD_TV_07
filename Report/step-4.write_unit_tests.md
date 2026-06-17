# TDD TV Channel Controller — 단위 테스트 작성 및 Green 구현

| 항목 | 내용 |
|------|------|
| 단계 | Step 4 — 단위 테스트 작성 (TDD Green) |
| 역할 | 시니어 C++ QA |
| 프로젝트 | TDD TV Channel Controller (C++17, CMake, Google Test / GMock) |
| 기준 문서 | `README.md` TO-DO 28건, `Report/step-1.requirements_analysis.md`, `Report/step-3.test_plan.md` |
| 산출물 | `test/*.cpp` 3종, `src/TVChannelController.cpp` 구현 |
| 검증 명령 | `cmake --build build && ctest --test-dir build` |
| 작성일 | 2026-05-19 |
| 선행 단계 | Step 3 테스트 계획 (T-01~T-28 정의) |
| 후속 단계 | Step 5~7 디버깅·리팩토링·Golden Master (README dev/refactoring) |

---

## 1. 요약

Step 3에서 정의한 **28개 시나리오(T-01~T-28)** 에 대응하는 단위 테스트를 작성하고, `TVChannelController` 프로덕션 코드를 구현하여 **전 테스트 Green** 상태를 달성했다.

| 구분 | Step 3 (계획) | Step 4 (완료) |
|------|---------------|---------------|
| `FakeTunerTest.cpp` | 6건 계획, 1 스텁 | **6건** 구현 |
| `TVChannelControllerTest.cpp` | 17건 계획, 1 스텁 | **17건** (`TEST_F`) |
| `TVChannelControllerMockTest.cpp` | 5건 계획, 1 스텁 | **5건** (`TEST_F`) |
| `TVChannelController.cpp` | 전 메서드 스텁 | **기능 구현 완료** |
| ctest 결과 | — | **28/28 Passed** |

---

## 2. 구현 개요 (`TVChannelController.cpp`)

### 2.1 책임 분리

| 메서드 | 동작 요약 |
|--------|-----------|
| `applyChannel(int ch)` | `0 ≤ ch ≤ 99` 검증 후 `tuner_.setCH(std::to_string(ch))`. 무효 시 `std::invalid_argument` |
| `pressNumber(int digit)` | 입력 버퍼 상태 머신: 한 자리 버퍼 → 두 자리 조합·적용·예외 |
| `pressConfirm()` | 버퍼에 값이 있으면 `applyChannel` 후 `inputBuffer_ = -1` |
| `pressOther()` | `inputBuffer_ = -1`만 수행 (튜너 미호출) |
| `pressFavorite()` | `getCurrentCH()` 기준 현재 채널을 `favorites_`에 토글 (추가 시 정렬) |
| `pressNextFavorite()` | `upper_bound` + wrap-around; 빈 목록이면 무동작 |

### 2.2 `pressNumber` 상태 머신 (핵심 설계)

`inputBuffer_`는 **-1(Idle)**, **0~9(한 자리)**, **10~99(두 자리 보류)** 를 표현한다.

```text
Idle (-1)
  └─ pressNumber(d) → buffer = d

한 자리 (0~9)
  └─ pressNumber(d2) → next = buffer*10 + d2
        · next > 99        → invalid_argument (T-14)
        · next < 10        → applyChannel(next), Idle  (T-11: 0,7 → 7)
        · next == 10       → buffer = 10 (세 번째 자리 대기)
        · 10 < next ≤ 99   → applyChannel(next), Idle  (T-08, T-09, T-13)

두 자리 보류 (10~99)
  └─ pressNumber(d3) → next = buffer*10 + d3
        · next > 99 → invalid_argument
        · else      → applyChannel(next), Idle
```

**T-14 (`1`, `0`, `0` → 100 예외)** 는 `1`+`0`에서 `next=10`을 즉시 적용하지 않고 버퍼에 보류한 뒤, 세 번째 `0`에서 `100`을 계산해 예외를 발생시키는 경로로 충족한다.  
**T-08·T-09** 등 `12`, `34`, `45` 등은 `10 < next ≤ 99` 분기에서 **즉시 적용**한다.

### 2명령 검증 결과

```text
cmake --build build
ctest --test-dir build --output-on-failure

100% tests passed, 0 tests failed out of 28
Total Test time (real) ≈ 0.5 sec
```

| CMake 타겟 | 테스트 수 | 결과 |
|------------|-----------|------|
| `fake_tuner_test` | 6 | Pass |
| `controller_test` | 17 | Pass |
| `controller_mock_test` | 5 | Pass |

---

## 3. 테스트 작성 규칙 준수

| 규칙 | 적용 내용 |
|------|-----------|
| Given-When-Then | 모든 테스트에 `// Given` / `// When` / `// Then` 주석 |
| 테스트 이름 | `동작_조건_기대결과` 패턴 (한글 스네이크) |
| `TEST` vs `TEST_F` | Fake 6건 `TEST`, Controller·Mock 각 `TEST_F` 픽스처 |
| 상태 검증 | `EXPECT_EQ` — `tuner.getCurrentCH()`, `ctrl.getFavoriteChannels()` |
| 예외 검증 | `EXPECT_THROW(..., std::invalid_argument)` — T-03, T-04, T-14 |
| 행위 검증 | `EXPECT_CALL`, `InSequence`, `.Times(1)` / `.Times(0)` — Mock 5건 |
| 경계값 | 채널 0/99/-1/100, digit 0·9, 빈·단일 favorites, wrap-around |

### 3.1 픽스처 (`ControllerTest`)

```cpp
class ControllerTest : public ::testing::Test {
protected:
    FakeTuner tuner{std::vector<int>{1, 4, 12, 56}};
    TVChannelController ctrl{tuner};

    void SetUp() override { tuner.setCH("0"); }

    void tuneTo(int ch);  // 복합 시퀀스 Arrange용 헬퍼
};
```

- `SetUp()`에서 초기 채널 `"0"` 고정 → T-07, T-12 등 반복 Arrange 제거
- `tuneTo(int)` — T-17 복합 토글 시퀀스에서 채널 이동 후 `pressFavorite()` 호출 지원

---

## 4. README TO-DO 추적 매트릭스 (T-01 ~ T-28)

### 4.1 FakeTunerTest.cpp (6건)

| No | 테스트명 | 검증 내용 | 결과 |
|----|----------|-----------|------|
| T-01 | `setCH_0_최솟값경계_현재채널0` | `setCH("0")` → `"0"` | Pass |
| T-02 | `setCH_99_최댓값경계_현재채널99` | `setCH("99")` → `"99"` | Pass |
| T-03 | `setCH_minus1_무효하한_예외발생` | `setCH("-1")` → `invalid_argument` | Pass |
| T-04 | `setCH_100_무효상한_예외발생` | `setCH("100")` → `invalid_argument` | Pass |
| T-05 | `seekCH_연속호출_항상유효채널반환` | 8회 연속 `0~99` | Pass |
| T-06 | `seekCH_목록끝_첫채널로wraparound` | `56` → `seekCH()` → `"1"` | Pass |

### 4.2 TVChannelControllerTest.cpp — 기능 1 (8건)

| No | 테스트명 | Act | Assert | 결과 |
|----|----------|-----|--------|------|
| T-07 | `한자리입력_확정_채널1로변경` | `1` + Confirm | `"1"` | Pass |
| T-08 | `두자리입력_즉시적용_채널12로변경` | `1`, `2` | `"12"` | Pass |
| T-09 | `연속입력_두번의두자리_12후34로변경` | `1,2,3,4` | `"34"` | Pass |
| T-10 | `버퍼입력후Other_45유지_6미적용` | `4,5,6` + Other | `"45"` | Pass |
| T-11 | `선행0입력_07_채널7로변경` | `0`, `7` | `"7"` | Pass |
| T-12 | `채널0입력_확정_채널0유지` | `0` + Confirm | `"0"` | Pass |
| T-13 | `채널99입력_두자리_채널99로변경` | `9`, `9` | `"99"` | Pass |
| T-14 | `채널100이상_입력_예외발생` | `1`, `0`, `0` | `invalid_argument` | Pass |

### 4.3 TVChannelControllerTest.cpp — 기능 2 (4건)

| No | 테스트명 | 검증 | 결과 |
|----|----------|------|------|
| T-15 | `미등록채널_즐겨찾기_목록에추가` | 채널 5 토글 → `{5}` | Pass |
| T-16 | `등록채널_즐겨찾기_목록에서삭제` | 이중 토글 → `{}` | Pass |
| T-17 | `복합토글시퀀스_목록637오름차순` | `12→★ 08→★ 37→★ 08→★ 06→★` → `{6,12,37}` | Pass |
| T-18 | `즐겨찾기목록_여러추가_오름차순정렬` | `addFavorite` 후 `{5,15,30}` | Pass |

### 4.4 TVChannelControllerTest.cpp — 기능 3 (5건)

| No | 테스트명 | Arrange | Assert | 결과 |
|----|----------|---------|--------|------|
| T-19 | `다음선호_현재6_채널12로이동` | fav `{1,4,12,56}`, 현재 `6` | `"12"` | Pass |
| T-20 | `다음선호_현재56_채널1로wraparound` | 동일, 현재 `56` | `"1"` | Pass |
| T-21 | `빈즐겨찾기_다음선호_채널변화없음` | fav `{}` | 채널 유지 | Pass |
| T-22 | `목록외현재채널_다음선호_첫큰값으로이동` | fav `{10,20}`, 현재 `5` | `"10"` | Pass |
| T-23 | `단일즐겨찾기_다음선호_wraparound동일채널` | fav `{7}`, 현재 `7` | `"7"` | Pass |

### 4.5 TVChannelControllerMockTest.cpp (5건)

| No | 테스트명 | Expectation | 결과 |
|----|----------|-------------|------|
| T-24 | `한자리확정_setCH_1회호출` | `setCH("1")` ×1 | Pass |
| T-25 | `두자리입력_setCH_12_1회호출` | `setCH("12")` ×1 | Pass |
| T-26 | `즐겨찾기토글_getCurrentCH_호출됨` | `getCurrentCH()` ≥1 | Pass |
| T-27 | `다음선호_목록있음_getCurrentCH후setCH_순서검증` | `InSequence`: get → set `"12"` | Pass |
| T-28 | `빈즐겨찾기_다음선호_setCH_미호출` | `setCH(_)` ×0 | Pass |

---

## 5. Fake vs Mock 검증 분리 (실행 결과)

| 구분 | 파일 | 검증 대상 | 사용 매크로·패턴 |
|------|------|-----------|------------------|
| **상태 기반** | `TVChannelControllerTest.cpp` | 최종 채널·즐겨찾기 목록 | `EXPECT_EQ`, `ASSERT_EQ`, `EXPECT_THROW` |
| **행위 기반** | `TVChannelControllerMockTest.cpp` | `ITuner` 협력 계약 | `EXPECT_CALL`, `InSequence`, `Times` |
| **Fake 단독** | `FakeTunerTest.cpp` | `FakeTuner` 경계·`seekCH` | `EXPECT_EQ`, `EXPECT_THROW` |

Mock 테스트는 **튜너 호출 여부·횟수·인자·순서**만 검증하고, 비즈니스 결과(최종 채널·목록)는 Fake 통합 테스트에 위임한다.

---

## 6. 변경·신규 파일 목록

| 경로 | 변경 유형 | 설명 |
|------|-----------|------|
| `src/TVChannelController.cpp` | 구현 | 스텁 → 전 public API + `applyChannel` |
| `test/FakeTunerTest.cpp` | 신규 테스트 | 스텁 1건 → T-01~T-06 (6건) |
| `test/TVChannelControllerTest.cpp` | 신규 테스트 | 스텁 1건 → T-07~T-23 (17건) |
| `test/TVChannelControllerMockTest.cpp` | 신규 테스트 | 스텁 1건 → T-24~T-28 (5건) |
| `include/FakeTuner.h` | 변경 없음 | Step 0~2에서 완료 |
| `include/MockTuner.h` | 변경 없음 | `MOCK_METHOD` 3개 |
| `include/TVChannelController.h` | 변경 없음 | 멤버·헬퍼 선언 유지 |
| `include/ITuner.h` | 변경 없음 | 수정 불가 계약 |
| `CMakeLists.txt` | 변경 없음 | 3 타겟·`gtest_discover_tests` 기등록 |

---

## 7. 설계 결정·주의 사항

### 7.1 채널 10 입력 (`1` + `0`)

`next == 10`일 때만 버퍼에 보류하고, `pressConfirm()`으로 채널 10 적용이 가능하다.  
README에 채널 10 단독 시나리오는 없으나, T-14(세 자리 시도)와 T-08~T-09(즉시 적용)를 동시에 만족하기 위한 분기이다.

### 7.2 `addFavorite(int)` 테스트 사용

T-18·T-19~T-23 Arrange에서 `ctrl.addFavorite()`를 사용한다.  
프로덕션 경로는 `pressFavorite()`이며, Step 2에서 지적한 **API 중복**은 README **dev/refactoring** 단계에서 `addToFavorites()` 추출로 정리 예정이다.

### 7.3 커버리지

본 단계에서는 **기능 Green**에 집중하였으며, gcov/lcov 80%+ 측정은 수행하지 않았다.  
리팩토링 브랜치(Step 6~7)에서 `clearBuffer()`, `findNextFavorite()` 추출 후 lcov 재측정을 권장한다.

---

## 8. 후속 작업 (README dev/refactoring)

Green 유지 상태에서 아래 항목을 진행한다.

| 우선순위 | 항목 | 근거 |
|----------|------|------|
| 1 | `clearBuffer()` 추출 | `pressOther` / `pressConfirm` / 두 자리 완성 후 버퍼 초기화 중복 |
| 2 | `findNextFavorite()` 추출 | `pressNextFavorite` 가독성·단위 검증 |
| 3 | `addToFavorites()` / `removeFromFavorites()` | `addFavorite` vs `pressFavorite` 중복 제거 |
| 4 | `ControllerTest::SetUp`·Given-When-Then 일관성 점검 | 테스트 가독성 |
| 5 | lcov ≥ 80% (권장 90%) | README 커버리지 목표 |

---

## 9. 결론

- README TO-DO **28개 테스트 시나리오**를 코드로 구현하고 **전건 Pass**하였다.
- `TVChannelController`는 `applyChannel` 단일 진입점을 통해 채널 변경·예외 계약을 `FakeTuner`와 일치시켰다.
- Fake(상태)·Mock(행위) 검증 전략을 분리하여 Step 3 테스트 계획을 실행 단계까지 완료했다.
- 다음 단계는 **리팩토링·커버리지·Golden Master**이며, 본 보고서의 Green 상태를 회귀 기준선으로 사용한다.
