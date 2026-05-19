# Step 5A — Debug test failures (Export Transcript)
_Exported on Tuesday, May 19, 2026 from Cursor_

---

## Turn 1

**User**

@step-5A_debug-test-failures.md (1-15)

**과제 요약 (tasks/step-5A_debug-test-failures.md)**

- [P] 디버깅과 결함 분석에 능한 C++ QA 엔지니어
- [C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
- [T] `ctest --test-dir build` 또는 `ctest --output-on-failure` 실패 로그 분석
  1. EXPECT_EQ / ASSERT_* / EXPECT_CALL 실패의 기대·실제 차이 요약
  2. 결함 위치 특정 (파일명:줄번호)
  3. 결함 심각도 (Critical/Major/Minor/Info) 분류 및 근거
  4. 최소 변경 수정 방안 (C++17, `ITuner.h` 수정 금지, Test Double·시나리오 의도 유지)
- [F] 수정 diff 제안 + `cmake --build build && ctest --test-dir build` Green 확인 절차

**참조 파일:** `README.md`, `tasks/step-1_requirements-analysis.md`, `include/ITuner.h`, `include/TVChannelController.h`, `src/TVChannelController.cpp`, `include/FakeTuner.h`, `test/FakeTunerTest.cpp`, `test/TVChannelControllerTest.cpp`, `test/TVChannelControllerMockTest.cpp`

---

**Cursor**

Step 5A 디버깅·결함 분석을 수행했습니다. ctest·소스·테스트·LastTest.log를 조사했습니다.

**수행한 작업**

1. **ctest 실행**
   - `ctest --test-dir build --output-on-failure` → **28/28 Passed**
   - `build/Testing/Temporary/LastTest.log` 확인 → 개별 케이스마다 **`Running 0 tests`** (한글 `gtest_filter` 깨짐)

2. **실행 파일 직접 실행**
   - `fake_tuner_test.exe` — **6/6 PASSED**
   - `controller_test.exe` — **17/17 PASSED**
   - `controller_mock_test.exe` — **5/5 PASSED**
   - **프로덕션 코드 결함 없음** (Step 4 Green 유지)

3. **결함 분석**
   - **D-01 [Critical]** — `CMakeLists.txt` `gtest_discover_tests` + Windows 비ASCII 테스트명 → CTest 거짓 Green
   - 스텁 시대(`ec52f15`) 예상 Red 패턴 회귀 참고용 정리 (채널 `"0"` 유지, `favorites_` 빈 목록, Mock `setCH` 0회)

4. **Report 작성**
   - `Report/step-5A.debug_test_failures.md` 저장

5. **CMake 수정 (D-01)**
   - Windows: 실행 파일 단위 `add_test` 3건
   - non-Windows: 기존 `gtest_discover_tests` 유지

6. **Green 재확인**
   - `cmake -S . -B build && cmake --build build`
   - `ctest --test-dir build` → **3/3 Passed** (내부 6+17+5=28 GTest 실제 실행)

**응답 요약 (Turn 1)**

| 구분 | 결과 |
|------|------|
| ctest (수정 전) | 28/28 Passed — **0 tests ran ×28 (거짓 Green)** |
| `*_test.exe` 직접 실행 | **28/28 실제 Pass** |
| `TVChannelController.cpp` | **결함 없음** |
| `FakeTuner.h` | **결함 없음** |
| 수정 파일 | `CMakeLists.txt` (Windows CTest 등록) |
| Report | `Report/step-5A.debug_test_failures.md` |

**핵심 발견: CTest 거짓 Green**

```text
# LastTest.log (수정 전)
Note: Google Test filter = FakeTunerTest.setCH_0_Ã¬ÂµÂœ...  (한글 깨짐)
[==========] Running 0 tests from 0 test suites.
[  PASSED  ] 0 tests.
```

| 항목 | 기대 | 실제 (수정 전) |
|------|------|----------------|
| ctest 개별 테스트 | GTest 1건 assert | 필터 불일치 → **0건 실행** |
| 종료 코드 | 실패 시 non-zero | **0건 실행도 exit 0** |

**스텁 대비 예상 Red (회귀 참고, 이미 Step 4 해소)**

| 테스트 | 기대 | 스텁 실제 | 실패 유형 |
|--------|------|-----------|-----------|
| `한자리입력_확정_채널1로변경` | `"1"` | `"0"` | `EXPECT_EQ` |
| `미등록채널_즐겨찾기_목록에추가` | `{5}` | `{}` | `ASSERT_EQ` size |
| `한자리확정_setCH_1회호출` | `setCH("1")` ×1 | 0회 | `EXPECT_CALL` |

**CMake 수정 diff 요약**

```cmake
include(GoogleTest)
if(WIN32)
    add_test(NAME fake_tuner_test COMMAND fake_tuner_test)
    add_test(NAME controller_test COMMAND controller_test)
    add_test(NAME controller_mock_test COMMAND controller_mock_test)
else()
    gtest_discover_tests(fake_tuner_test)
    gtest_discover_tests(controller_test)
    gtest_discover_tests(controller_mock_test)
endif()
```

**Green 확인 절차**

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
# 3/3 Passed, LastTest.log에 6+17+5 tests ran 확인
```

**심각도 분류**

| ID | Severity | Area | 내용 |
|----|----------|------|------|
| D-01 | **Critical** | CMake/CTest | 검증 미수행인데 Pass — CI 게이트 무력화 |
| D-02 | **Info** | 문서/프로세스 | Step 4 Report ctest 28/28 vs 실제 의미 불일치 |
| (스텁) | Major | 기능1~3 | Step 4에서 이미 해소 |

**후속:** Step 5B `docs/defect_list.md`에 D-01·D-02 반영

---

## Turn 2

**User**

현재까지의 대화내용 전체를 Prompting 폴더에 Export Transcript 해 주세요. 파일명은 step- 으로 시작

---

**Cursor**

`Prompting/step-5A-debug-test-failures-prompt.md`에 Export Transcript를 저장했습니다. (본 파일)

도구 호출·내부 로그·긴 소스 덤프는 제외하고, 과제 지시·수행 작업·산출물·응답 요약 위주로 읽기용 transcript로 정리했습니다.

---

## 부록: 주요 파일 경로

| 경로 | 설명 |
|------|------|
| `Report/step-5A.debug_test_failures.md` | Step 5A 결함 분석 Report |
| `CMakeLists.txt` | Windows CTest 등록 수정 (D-01) |
| `src/TVChannelController.cpp` | 프로덕션 — 결함 없음 (28/28 Green) |
| `include/FakeTuner.h` | Test Double — 결함 없음 |
| `test/TVChannelControllerTest.cpp` | 상태 기반 17건 |
| `test/TVChannelControllerMockTest.cpp` | Mock 행위 5건 |
| `test/FakeTunerTest.cpp` | FakeTuner 6건 |
| `tasks/step-5A_debug-test-failures.md` | 과제 지시문 |
| `tasks/step-5B_defect-list.md` | 후속 — 결함 목록 Markdown |
| `Prompting/step-5A-debug-test-failures-prompt.md` | 본 Export Transcript |

## 부록: ctest · 직접 실행 비교

| 검증 방식 | 수정 전 | 수정 후 |
|-----------|---------|---------|
| `ctest --test-dir build` | 28/28 (각 0 tests ran) | **3/3** (exe 전체 스위트) |
| `fake_tuner_test.exe` | 6/6 | 6/6 |
| `controller_test.exe` | 17/17 | 17/17 |
| `controller_mock_test.exe` | 5/5 | 5/5 |
| **실질 GTest 합계** | 28 (직접 실행만 신뢰) | **28 (ctest로도 신뢰)** |

```text
cmake --build build
ctest --test-dir build --output-on-failure
```

## 부록: 결함 목록 (D-01, D-02)

### D-01 [Critical] CTest 한글 필터 거짓 Pass

| 필드 | 내용 |
|------|------|
| **Area** | CMake / CTest |
| **Steps** | `ctest --test-dir build` |
| **Expected** | 28 GTest 각각 실행·assert |
| **Actual** | 필터 깨짐 → 0 tests ran, ctest Passed |
| **Root Cause** | `gtest_discover_tests` + Windows 비ASCII `--gtest_filter` |
| **Fix** | Windows `add_test` 3건 (실행 파일 단위) |
| **위치** | `CMakeLists.txt` |

### D-02 [Info] Step 4 Report와 ctest 의미 불일치

| 필드 | 내용 |
|------|------|
| **Expected** | ctest 28/28 = 28 시나리오 검증 |
| **Actual** | 직접 exe만 실제 검증, ctest는 거짓 Green |
| **Fix** | D-01 적용 후 ctest 3/3 = 실제 28건 실행 |

## 부록: 프로덕션 구현 ↔ 테스트 매핑 (결함 없음 확인)

| ID | 시나리오 | 구현 근거 (`TVChannelController.cpp`) |
|----|----------|--------------------------------------|
| T-08~09 | 두 자리 즉시 적용 | `10 < next ≤ 99` → `applyChannel` |
| T-11 | `0,7` → 7 | `next < 10` → `applyChannel` |
| T-14 | `1,0,0` → 예외 | `next==10` 버퍼 보류 → 세 번째 자리 `>99` |
| T-10 | Other 후 6 미적용 | `pressOther` → `inputBuffer_=-1` |
| T-19~20 | next favorite + wrap | `upper_bound` + `favorites_.front()` |

`ITuner.h` — **미수정** (외부 인터페이스 준수).

## 부록: `pressNumber` 상태 머신 (Step 4 구현, Step 5A 변경 없음)

```text
Idle (-1)
  └─ pressNumber(d) → buffer = d

한 자리 (0~9)
  └─ pressNumber(d2) → next = buffer*10 + d2
        · next > 99        → invalid_argument (T-14)
        · next < 10        → applyChannel(next), Idle  (T-11)
        · next == 10       → buffer = 10 (세 번째 자리 대기)
        · 10 < next ≤ 99   → applyChannel(next), Idle

두 자리 보류 (10~99)
  └─ pressNumber(d3) → next = buffer*10 + d3
        · next > 99 → invalid_argument
        · else      → applyChannel(next), Idle
```

## 부록: Mock 실패 패턴 (스텁 시대 참고)

| 테스트 | EXPECT_CALL | 스텁 시 실제 |
|--------|-------------|--------------|
| `한자리확정_setCH_1회호출` | `setCH("1")` ×1 | 0회 |
| `두자리입력_setCH_12_1회호출` | `setCH("12")` ×1 | 0회 |
| `다음선호_목록있음_getCurrentCH후setCH_순서검증` | `getCurrentCH` → `setCH("12")` | 미호출 |
| `빈즐겨찾기_다음선호_setCH_미호출` | `setCH` ×0 | (우연 통과 가능) |

## 부록: Green 확인 절차 (Step 5A 최종)

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

.\build\fake_tuner_test.exe
.\build\controller_test.exe
.\build\controller_mock_test.exe
```

| 명령 | 결과 |
|------|------|
| `ctest` | 3/3 Passed |
| 직접 exe | 28/28 Passed |
| `ITuner.h` | 변경 없음 |

## 부록: Prompting · Report 시리즈

| 파일 | 단계 |
|------|------|
| `step-1-requirements-analysis-prompt.md` | Step 1 |
| `step-2-code-quality-prompt.md` | Step 2 |
| `step-3-test-plan-prompt.md` | Step 3 |
| `step-4-write-unit-tests-prompt.md` | Step 4 |
| `step-5A-debug-test-failures-prompt.md` | Step 5A (본 파일) |
| `Report/step-5A.debug_test_failures.md` | Step 5A Report |

## 부록: 후속 단계

| Step | 내용 |
|------|------|
| Step 5B | `docs/defect_list.md` — D-01·D-02 형식 정리 |
| Step 6 | Golden Master |
| Step 7 | 리팩토링 (`clearBuffer`, `findNextFavorite`, lcov) |
