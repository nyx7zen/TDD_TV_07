# Step 5A — 테스트 실패 디버깅·결함 분석

| 항목 | 내용 |
|------|------|
| 단계 | Step 5A — 디버깅·결함 분석 |
| 역할 | C++ QA 엔지니어 |
| 검증 일시 | 2026-05-19 |
| ctest | `ctest --test-dir build --output-on-failure` → **28/28 Passed** (아래 §1.1 참고) |
| 직접 실행 | `build/*_test.exe` → **28/28 Passed** (실제 검증) |

---

## 1. 테스트 실행 요약

### 1.1 ctest — 거짓 Green (Critical 인프라 결함)

`ctest`는 28건 모두 **Passed**이나, `build/Testing/Temporary/LastTest.log`를 보면 **개별 필터마다 0 tests ran** 이다.

```text
Note: Google Test filter = FakeTunerTest.setCH_0_Ã¬ÂµÂœÃ¬Â†ÂŸ...  (한글 깨짐)
[==========] Running 0 tests from 0 test suites.
[  PASSED  ] 0 tests.
```

| 항목 | 기대 | 실제 |
|------|------|------|
| ctest 개별 테스트 | 해당 GTest 1건 실행·검증 | **필터 불일치 → 0건 실행** |
| 종료 코드 | 실패 시 non-zero | **0건 실행도 exit 0 → Passed** |
| 영향 | 28 시나리오 검증 | **시나리오 미실행인데 Green으로 오인** |

**근본 원인:** `gtest_discover_tests`가 생성한 `--gtest_filter`에 한글 테스트명이 포함되는데, Windows CTest → gtest 전달 시 **인코딩 불일치**(UTF-8 vs 시스템 코드 페이지)로 필터 문자열이 깨진다.

### 1.2 실행 파일 직접 실행 — 실제 Green

```powershell
.\build\fake_tuner_test.exe          # 6/6  PASSED
.\build\controller_test.exe          # 17/17 PASSED
.\build\controller_mock_test.exe     # 5/5 PASSED
```

**결론:** `TVChannelController`, `FakeTuner`, Mock 협력 계약은 **현재 구현 기준 결함 없음**. 수정 대상은 **CMake/CTest 등록 방식**이다.

---

## 2. 스텁 대비 Red 단계에서 예상되던 실패 (회귀 참고)

초기 스텁(`ec52f15`)은 모든 메서드가 빈 구현이었다. Step 4 테스트 추가 직후 예상 실패 패턴:

### 2.1 기능 1 — 숫자 입력 (`TVChannelControllerTest.cpp`)

| 테스트 | 기대 | 스텁 실제 | 실패 유형 |
|--------|------|-----------|-----------|
| `한자리입력_확정_채널1로변경` | `"1"` | `"0"` | `EXPECT_EQ` 채널 문자열 |
| `두자리입력_즉시적용_채널12로변경` | `"12"` | `"0"` | `EXPECT_EQ` |
| `연속입력_두번의두자리_12후34로변경` | `"34"` | `"0"` | `EXPECT_EQ` |
| `버퍼입력후Other_45유지_6미적용` | `"45"` | `"0"` | `EXPECT_EQ` |
| `선행0입력_07_채널7로변경` | `"7"` | `"0"` | `EXPECT_EQ` |
| `채널100이상_입력_예외발생` | `invalid_argument` | 예외 없음 | `EXPECT_THROW` 미발생 |

**결함 위치(스텁):** `src/TVChannelController.cpp` — `pressNumber`, `pressConfirm`, `applyChannel` 전부 빈 구현.

### 2.2 기능 2·3 — 선호 채널

| 테스트 | 기대 | 스텁 실제 |
|--------|------|-----------|
| `미등록채널_즐겨찾기_목록에추가` | `favorites_ == {5}` | `{}` |
| `다음선호_현재6_채널12로이동` | `"12"` | `"6"` |

**결함 위치(스텁):** `pressFavorite`, `pressNextFavorite` 빈 구현.

### 2.3 Mock 검증 (`TVChannelControllerMockTest.cpp`)

| 테스트 | 기대 | 스텁 실제 |
|--------|------|-----------|
| `한자리확정_setCH_1회호출` | `setCH("1")` ×1 | **0회** — `MockFunctionCalledWrongNumberOfTimes` |
| `빈즐겨찾기_다음선호_setCH_미호출` | `setCH` ×0 | (스텁은 0회라 우연 통과 가능) |

---

## 3. 현재 구현 — 결함 없음 (검증 완료)

Step 4 Green 구현 후 **직접 gtest 실행 기준** 실패 로그 없음. 주요 시나리오와 구현 매핑:

| ID | 시나리오 | 구현 근거 (`TVChannelController.cpp`) |
|----|----------|--------------------------------------|
| T-08~09 | 두 자리 즉시 적용 | `10 < next ≤ 99` → `applyChannel` (L26-28) |
| T-11 | `0,7` → 7 | `next < 10` → `applyChannel` (L21-23) |
| T-14 | `1,0,0` → 예외 | `next==10` 버퍼 보류 후 세 번째 자리에서 `>99` (L24-25, L33-35) |
| T-10 | Other 후 6 미적용 | `pressOther` → `inputBuffer_=-1` (L69-71) |
| T-19~20 | next favorite + wrap | `upper_bound` + `favorites_.front()` (L63-66) |

`ITuner.h` — **미수정** (요구사항 준수).

---

## 4. 발견 결함 목록 (현재 워크스페이스)

### D-01 [Critical] [Controller공통 / CMake] CTest 한글 필터 거짓 Pass

| 필드 | 내용 |
|------|------|
| **Steps** | `cmake --build build && ctest --test-dir build` |
| **Expected** | 28개 GTest 각각 실행·assert |
| **Actual** | 필터 깨짐 → **0 tests ran** ×28, ctest는 Passed |
| **Root Cause** | `gtest_discover_tests` + Windows 비ASCII `gtest_filter` 인코딩 |
| **Fix Summary** | Windows에서 실행 파일 단위 `add_test` 3건으로 등록 (§5) |
| **위치** | `CMakeLists.txt` L33-36 |

### D-02 [Info] Step 4 산출물과 ctest 보고 불일치

| 필드 | 내용 |
|------|------|
| **Expected** | ctest 28/28 = 실제 28 시나리오 검증 |
| **Actual** | Step 4 Report의 Green은 **직접 exe 실행**과 일치, **ctest만** 거짓 Green |
| **Fix Summary** | D-01 수정 후 ctest 재실행·Report 정정 |

**프로덕션 코드(`TVChannelController.cpp`, `FakeTuner.h`) 결함: 없음.**

---

## 5. 최소 변경 수정안

### 5.1 CMakeLists.txt (D-01)

Windows에서는 per-test discovery 대신 **실행 파일 단위** 3 테스트로 등록한다. Linux/macOS는 기존 `gtest_discover_tests` 유지 가능.

```cmake
if(WIN32)
    add_test(NAME fake_tuner_test COMMAND fake_tuner_test)
    add_test(NAME controller_test COMMAND controller_test)
    add_test(NAME controller_mock_test COMMAND controller_mock_test)
else()
    include(GoogleTest)
    gtest_discover_tests(fake_tuner_test)
    gtest_discover_tests(controller_test)
    gtest_discover_tests(controller_mock_test)
endif()
```

`include(GoogleTest)`는 `if(WIN32)` 밖에 두고, non-Windows 분기에서만 discover 호출.

### 5.2 프로덕션 코드

**변경 불필요** — 직접 실행 28/28 Green.

---

## 6. Green 확인 절차

```powershell
cmake --build build
ctest --test-dir build --output-on-failure

# ctest 수정 후: 3/3 Passed (각 exe 내부 6+17+5=28건)
.\build\fake_tuner_test.exe
.\build\controller_test.exe
.\build\controller_mock_test.exe
```

| 명령 | 수정 전 | 수정 후 (목표) |
|------|---------|----------------|
| `ctest` | 28/28 (0건씩 실행) | **3/3** (exe당 전체 스위트 실행) |
| `*_test.exe` | 28/28 | 28/28 |

---

## 7. 심각도 분류 요약

| ID | Severity | Area | 근거 |
|----|----------|------|------|
| D-01 | **Critical** | CMake/CTest | CI·로컬에서 **검증 미수행**인데 Pass — 품질 게이트 무력화 |
| D-02 | **Info** | 문서/프로세스 | Report와 ctest 의미 불일치, 혼동 유발 |
| (스텁 시대) | Major | 기능1~3 | 채널·즐겨찾기·Mock 전부 실패 — **이미 Step 4에서 해소** |

---

## 8. 후속

- **Step 5B:** `docs/defect_list.md`에 D-01·D-02 반영
- **Step 7:** `pressNumber` 상태 머신 리팩토링 (기능 변경 없음)
