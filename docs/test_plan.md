# TDD TV Channel Controller — 테스트 계획서

| 항목 | 내용 |
|------|------|
| 프로젝트 | TDD TV Channel Controller (C++17, CMake, GTest/GMock) |
| 기준 | `README.md` TO-DO 28건, `docs/requirements_analysis.md` (T-01~T-28) |
| 커버리지 목표 | README **80%+** (권장 **90%+**) |
| 측정 도구 | gcov/lcov (리팩토링 브랜치 README 항목) |

---

## 1. 단위 테스트 범위·우선순위

### 1.1 실행 파일·범위

| 순서 | CMake 타겟 | 소스 | 검증 대상 | Test Double |
|------|------------|------|-----------|-------------|
| 1 | `fake_tuner_test` | `FakeTunerTest.cpp` | `FakeTuner` 단독 | — (SUT 자체) |
| 2 | `controller_test` | `TVChannelControllerTest.cpp` | `TVChannelController` + 통합 | **FakeTuner** (상태) |
| 3 | `controller_mock_test` | `TVChannelControllerMockTest.cpp` | 컨트롤러–튜너 협력 | **MockTuner** (행위) |

### 1.2 TDD Red → Green 진행 순서

```text
Phase A: fake_tuner_test (T-01~T-06)
    → FakeTuner 경계·seekCH 계약 확정

Phase B: controller_test 기능 1 (T-07~T-14)
    → applyChannel / pressNumber / pressConfirm / pressOther

Phase C: controller_test 기능 2 (T-15~T-18)
    → pressFavorite, favorites_ 정렬

Phase D: controller_test 기능 3 (T-19~T-23)
    → pressNextFavorite, upper_bound, wrap

Phase E: controller_mock_test (T-24~T-28)
    → setCH 횟수·순서·미호출 (Green 유지하며 보완)
```

### 1.3 TEST vs TEST_F

| 파일 | 권장 패턴 | 이유 |
|------|-----------|------|
| `FakeTunerTest` | `TEST` 위주 | 상태less 단순 시나리오 |
| `TVChannelControllerTest` | **`TEST_F(ControllerTest, ...)`** | `FakeTuner` + `TVChannelController` 공통 Arrange (`SetUp`) |
| `TVChannelControllerMockTest` | **`TEST_F(MockControllerTest, ...)`** | `ON_CALL` 기본값, `InSequence` 공유 |

**픽스처 SetUp 예시 (리팩토링 TODO 반영):**

```cpp
class ControllerTest : public ::testing::Test {
protected:
    FakeTuner tuner{std::vector<int>{1, 4, 12, 56}};
    TVChannelController ctrl{tuner};
    void SetUp() override { tuner.setCH("0"); }
};
```

---

## 2. 경계값 케이스 목록

| ID | 카테고리 | 입력·상태 | 기대 | 매핑 |
|----|----------|-----------|------|------|
| B-01 | 채널 하한 | `0` | 정상 적용 | T-01, T-07, T-12 |
| B-02 | 채널 상한 | `99` | 정상 적용 | T-02, T-13 |
| B-03 | 채널 직전 | `98` | 정상 (구현 시 T-12/13 보완) | 확장 |
| B-04 | 채널 직후 | `1` | 한 자리·두 자리 | T-07, T-08 |
| B-05 | 무효 하한 | `-1` (`setCH`) | `invalid_argument` | T-03 |
| B-06 | 무효 상한 | `100` (`setCH` / `1,0,0`) | `invalid_argument` | T-04, T-14 |
| B-07 | digit 하한 | `0` | 유효 버튼 | T-11, T-12 |
| B-08 | digit 상한 | `9` | `99` = `9`,`9` | T-13 |
| B-09 | `inputBuffer_` Idle | `-1` | 확정 무동작 | F1-10 |
| B-10 | 선행 0 | `0`,`7` → `7` | `"7"` | T-11 |
| B-11 | 빈 `favorites_` | `{}` | next 무동작 | T-21, T-28 |
| B-12 | 단일 favorite | `{12}` | wrap 동일 채널 | T-23 |
| B-13 | seek wrap | `available_` 끝 | `front()` | T-06 |
| B-14 | next fav wrap | 현재 `56`, fav `{1,4,12,56}` | `"1"` | T-20 |

---

## 3. 예외·특이 케이스 목록

| ID | 시나리오 | 기대 | 테스트 |
|----|----------|------|--------|
| E-01 | `FakeTuner::setCH("-1")` | `std::invalid_argument` | T-03 |
| E-02 | `FakeTuner::setCH("100")` | `std::invalid_argument` | T-04 |
| E-03 | 컨트롤러 `1,0,0` | 예외 전파 | T-14 |
| E-04 | `pressOther` 버퍼만 클리어 | 튜너 채널 유지 | T-10 |
| E-05 | 현재 ∉ favorites | next = upper_bound 또는 wrap | T-22 |
| E-06 | 빈 favorites + `pressNextFavorite` | `setCH` 0회 (Mock) | T-21, T-28 |
| E-07 | `pressFavorite` | `getCurrentCH` 호출 (Mock) | T-26 |
| E-08 | `pressNextFavorite` 순서 | `getCurrentCH` → `setCH` | T-27 |
| E-09 | 두 자리 즉시 vs 확정 | `12` 즉시 / `1`+확정 | T-08, T-07 |
| E-10 | 복합 토글 시퀀스 | `{6,12,37}` | T-17 |

---

## 4. Fake vs Mock 검증 분리 전략

| 항목 | Fake (`controller_test`) | Mock (`controller_mock_test`) |
|------|--------------------------|-------------------------------|
| **검증 대상** | 최종 채널, favorites 목록, 예외 | `setCH`/`getCurrentCH` 호출·횟수·순서 |
| **Assert** | `EXPECT_EQ(tuner.getCurrentCH(), "...")` | `EXPECT_CALL(mock, setCH("12")).Times(1)` |
| **Arrange** | `FakeTuner({1,4,12,56})`, `setCH`로 현재 채널 | `ON_CALL(mock, getCurrentCH()).WillByDefault(Return("6"))` |
| **순서** | 불필요 | `InSequence` (T-27) |
| **미호출** | 채널 문자열 동일 | `EXPECT_CALL(setCH(_)).Times(0)` (T-28) |
| **중복 금지** | Mock에서 favorites 내용 검증 X | Fake에서 `Times(1)` 남용 X |

**원칙:** 동일 시나리오를 Fake·Mock에 **전부** 복제하지 않는다. 기능 1~3·경계·예외는 Fake; 튜너 **협력 계약** 5건만 Mock.

---

## 5. 커버리지 목표·gcov/lcov 전략

### 5.1 목표

| 구분 | 목표 | 비고 |
|------|------|------|
| README 명시 | **≥ 80%** 라인 | dev/refactoring 체크리스트 |
| 권장 | **≥ 90%** | `TVChannelController.cpp` 분기 |

### 5.2 측정 (CMake 확장 시)

```bash
# 예시: Release with coverage flags 추가 후
cmake -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage" ..
cmake --build .
ctest
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/test/*' '*/googletest/*' --output-file coverage.filtered.info
genhtml coverage.filtered.info --output-directory coverage_html
```

### 5.3 미커버 대응

| 우선 확인 라인 | 대응 테스트 |
|----------------|-------------|
| `pressConfirm` + 빈 버퍼 | E-09 / F1-10 |
| `pressNumber` digit 범위 외 | B-07 확장 |
| `pressFavorite` 제거 분기 | T-16 |
| `findNextFavorite` wrap | T-20, T-23 |
| `applyChannel` invalid | T-14 |

---

## 6. README TO-DO 28건 추적 매트릭스

| ID | 우선순위 | 테스트 파일 | 권장 테스트명 (동작_조건_기대) | README 항목 |
|----|----------|-------------|-------------------------------|-------------|
| T-01 | P0 | FakeTunerTest | `SetCH_MinBoundary_ReturnsZero` | setCH("0") |
| T-02 | P0 | FakeTunerTest | `SetCH_MaxBoundary_Returns99` | setCH("99") |
| T-03 | P0 | FakeTunerTest | `SetCH_Negative_ThrowsInvalidArgument` | setCH("-1") |
| T-04 | P0 | FakeTunerTest | `SetCH_100_ThrowsInvalidArgument` | setCH("100") |
| T-05 | P1 | FakeTunerTest | `SeekCH_Repeated_AlwaysValidChannel` | seek 연속 |
| T-06 | P1 | FakeTunerTest | `SeekCH_AtEnd_WrapsToFirst` | seek wrap |
| T-07 | P0 | ControllerTest | `PressNumber_ThenConfirm_SetsSingleDigit` | 1+확정→"1" |
| T-08 | P0 | ControllerTest | `PressNumber_Twice_AppliesTwoDigits` | 1,2→"12" |
| T-09 | P1 | ControllerTest | `PressNumber_FourDigits_Applies12Then34` | 1,2,3,4 |
| T-10 | P1 | ControllerTest | `PressOther_ClearsBuffer_KeepsChannel` | 4,5,6+Other |
| T-11 | P1 | ControllerTest | `PressNumber_LeadingZero_AppliesSeven` | 0,7→"7" |
| T-12 | P1 | ControllerTest | `PressNumber_ChannelZero_Applies` | 채널 0 |
| T-13 | P1 | ControllerTest | `PressNumber_Channel99_Applies` | 채널 99 |
| T-14 | P0 | ControllerTest | `PressNumber_Channel100_Throws` | 100+ 예외 |
| T-15 | P1 | ControllerTest | `PressFavorite_NotInList_Adds` | 미등록 추가 |
| T-16 | P1 | ControllerTest | `PressFavorite_InList_Removes` | 토글 삭제 |
| T-17 | P2 | ControllerTest | `PressFavorite_Sequence_FinalList637` | 복합 시퀀스 |
| T-18 | P1 | ControllerTest | `GetFavoriteChannels_AlwaysSorted` | 정렬 검증 |
| T-19 | P1 | ControllerTest | `PressNextFavorite_From6_GoesTo12` | {1,4,12,56}, cur 6 |
| T-20 | P1 | ControllerTest | `PressNextFavorite_From56_WrapsTo1` | wrap 끝 |
| T-21 | P1 | ControllerTest | `PressNextFavorite_EmptyList_NoChange` | 빈 목록 |
| T-22 | P2 | ControllerTest | `PressNextFavorite_CurrentNotInList_GoesNext` | 목록 외 |
| T-23 | P2 | ControllerTest | `PressNextFavorite_SingleItem_Wraps` | 1개 wrap |
| T-24 | P1 | MockControllerTest | `Confirm_CallsSetCHOnce` | setCH("1")×1 |
| T-25 | P1 | MockControllerTest | `TwoDigits_CallsSetCH12Once` | setCH("12")×1 |
| T-26 | P1 | MockControllerTest | `PressFavorite_CallsGetCurrentCH` | getCurrentCH |
| T-27 | P2 | MockControllerTest | `PressNextFavorite_CallsGetThenSet_InOrder` | 순서 |
| T-28 | P1 | MockControllerTest | `PressNextFavorite_Empty_NoSetCH` | setCH 미호출 |

**우선순위:** P0 = Fake 경계 + 핵심 입력·예외 → P1 = 나머지 기능·Mock → P2 = 복합·엣지.

---

## 7. 테스트 작성 체크리스트 (AAA)

- [ ] **Arrange:** 튜너 초기 채널·favorites 사전 설정 명시
- [ ] **Act:** SUT 메서드만 호출
- [ ] **Assert:** Fake=상태, Mock=EXPECT_CALL
- [ ] Given-When-Then 주석 (리팩토링 TODO)
- [ ] `ASSERT_*` = 전제, `EXPECT_*` = 결과
- [ ] CTest 3타겟 모두 Green 후 lcov 80%+

---

## 8. 현재 상태

| 항목 | 상태 |
|------|------|
| 테스트 구현 | 스텁 3건 (T-01~T-28 미착수) |
| 컨트롤러 구현 | 스텁 |
| 계획 대비 | 본 문서 기준 Red 단계 착수 가능 |

본 계획서는 `docs/requirements_analysis.md` T-01~T-28과 1:1 추적된다.
