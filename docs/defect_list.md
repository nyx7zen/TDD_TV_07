# Defect List — TDD TV Channel Controller

| 항목 | 내용 |
|------|------|
| 단계 | Step 5B — 결함 목록 |
| 역할 | QA 리드 |
| 작성 일시 | 2026-05-19 |
| 근거 | Step 5A 분석, `ctest` / 직접 gtest 실행, README 시나리오 대비 |
| **현재 오픈 결함** | **0건** (프로덕션·FakeTuner·기능1~3·Mock) |
| **해결됨 / 회귀 참고** | D-01(수정 완료), D-02(정보), D-H01~H08(스텁 시대) |

---

## 요약

| ID | Severity | Area | 상태 | 요약 |
|----|----------|------|------|------|
| D-01 | Critical | Controller공통 | **해결** | Windows CTest 한글 `gtest_filter` 거짓 Pass |
| D-02 | Info | Controller공통 | **정보** | Step 4 Report의 ctest 28/28과 실제 검증 의미 불일치 |
| D-H01 | Major | 기능1(숫자입력) | 해결 | 스텁: 한 자리 확정 미적용 |
| D-H02 | Major | 기능1(숫자입력) | 해결 | 스텁: 두 자리 즉시 적용 미적용 |
| D-H03 | Major | 기능1(숫자입력) | 해결 | 스텁: 연속 두 자리 입력 미적용 |
| D-H04 | Major | 기능1(숫자입력) | 해결 | 스텁: Other 후 버퍼 무효화 미구현 |
| D-H05 | Major | 기능1(숫자입력) | 해결 | 스텁: 선행 0 (`0,7` → 7) 미적용 |
| D-H06 | Major | 기능1(숫자입력) | 해결 | 스텁: 채널 100+ 예외 미발생 |
| D-H07 | Major | 기능2(선호토글) | 해결 | 스텁: `pressFavorite` 빈 구현 |
| D-H08 | Major | 기능3(다음선호) / Mock검증 | 해결 | 스텁: `pressNextFavorite` / `setCH` 미호출 |

**검증 (2026-05-19):** `ctest` 3/3 Passed · `fake_tuner_test` 6/6 · `controller_test` 17/17 · `controller_mock_test` 5/5

---

## 오픈 결함

없음. `TVChannelController.cpp`, `FakeTuner.h` 및 README·테스트 플랜 시나리오는 현재 구현·테스트 기준으로 일치한다.

---

## 결함 상세

### D-01 [Critical] [Controller공통] — Windows CTest 거짓 Green (해결됨)

| 필드 | 내용 |
|------|------|
| **Steps** | `cmake --build build` 후 `ctest --test-dir build --output-on-failure` (수정 전: `gtest_discover_tests` per-case 등록) |
| **Expected** | README·테스트 플랜 28개 GTest 각각 실행·assert |
| **Actual** | `LastTest.log`에 `Running 0 tests from 0 test suites` ×28, ctest는 **Passed** (필터 깨짐으로 미실행) |
| **Root Cause** | `gtest_discover_tests`가 생성한 `--gtest_filter`에 한글 테스트명 포함 → Windows에서 CTest→gtest 전달 시 UTF-8/시스템 코드 페이지 불일치로 필터 문자열 손상 |
| **Fix Summary** | `CMakeLists.txt`: Windows는 실행 파일 단위 `add_test` 3건, non-Windows는 `gtest_discover_tests` 유지 (L33–44) |
| **위치** | `CMakeLists.txt` |

---

### D-02 [Info] [Controller공통] — 문서·ctest 보고 불일치

| 필드 | 내용 |
|------|------|
| **Steps** | Step 4 Report의 Green 근거와 `ctest` 결과 대조 |
| **Expected** | ctest 28/28 = 28 시나리오 실제 검증 |
| **Actual** | D-01 시점 ctest는 시나리오 미실행인데 Passed; Green은 **직접 `*_test.exe` 실행**과 일치 |
| **Root Cause** | CI/로컬 품질 게이트가 ctest만 볼 때 D-01로 오인 가능 |
| **Fix Summary** | D-01 적용 후 ctest는 3/3(exe 단위, 내부 28건); Report·README에 “Windows ctest = 3 executable tests” 명시 권장 |

---

## 회귀 참고 — 스텁 시대 결함 (Step 4 이전, 모두 해결)

초기 스텁(`TVChannelController.cpp` 빈 구현)에서 Red 단계로 관측되던 패턴이다. 현재 구현·테스트는 Green.

### D-H01 [Major] [기능1(숫자입력)]

| 필드 | 내용 |
|------|------|
| **Steps** | `pressNumber(1)` → `pressConfirm()` |
| **Expected** | `getCurrentCH() == "1"` |
| **Actual** | `"0"` (`EXPECT_EQ` 실패) |
| **Root Cause** | `pressNumber` / `pressConfirm` / `applyChannel` 스텁 |
| **Fix Summary** | `pressConfirm`에서 버퍼 적용·`applyChannel` → `tuner_.setCH` (해결) |

### D-H02 [Major] [기능1(숫자입력)]

| 필드 | 내용 |
|------|------|
| **Steps** | `pressNumber(1)` → `pressNumber(2)` |
| **Expected** | `"12"` |
| **Actual** | `"0"` |
| **Root Cause** | 두 자리 즉시 적용 로직 부재 |
| **Fix Summary** | `10 < next ≤ 99` 시 `applyChannel` (L26–28) |

### D-H03 [Major] [기능1(숫자입력)]

| 필드 | 내용 |
|------|------|
| **Steps** | `1,2,3,4` 연속 입력 |
| **Expected** | 최종 `"34"` |
| **Actual** | `"0"` |
| **Root Cause** | 동일 — 버퍼·즉시 적용 미구현 |
| **Fix Summary** | D-H02와 동일 상태 머신 |

### D-H04 [Major] [기능1(숫자입력)]

| 필드 | 내용 |
|------|------|
| **Steps** | `4,5` 입력 후 `pressNumber(6)` → `pressOther()` |
| **Expected** | `"45"` (6 미적용) |
| **Actual** | `"0"` |
| **Root Cause** | `pressOther` 미구현 |
| **Fix Summary** | `pressOther` → `inputBuffer_ = -1` (L69–71) |

### D-H05 [Major] [기능1(숫자입력)]

| 필드 | 내용 |
|------|------|
| **Steps** | `pressNumber(0)` → `pressNumber(7)` |
| **Expected** | `"7"` |
| **Actual** | `"0"` |
| **Root Cause** | `next < 10` 단일 자리 즉시 적용 부재 |
| **Fix Summary** | `next < 10` → `applyChannel` (L21–23) |

### D-H06 [Major] [기능1(숫자입력)]

| 필드 | 내용 |
|------|------|
| **Steps** | `pressNumber(1)` → `0` → `0` |
| **Expected** | `std::invalid_argument` |
| **Actual** | 예외 없음 (`EXPECT_THROW` 실패) |
| **Root Cause** | 범위 검사·세 번째 자리 처리 부재 |
| **Fix Summary** | `next > 99` throw, `inputBuffer_==10` 후 세 자리 (L19–20, L33–35) |

### D-H07 [Major] [기능2(선호토글)]

| 필드 | 내용 |
|------|------|
| **Steps** | 채널 5로 이동 후 `pressFavorite()` |
| **Expected** | `getFavoriteChannels()`에 `5` 포함 |
| **Actual** | 빈 목록 |
| **Root Cause** | `pressFavorite` 스텁 |
| **Fix Summary** | 토글·정렬 (`pressFavorite` L47–57) |

### D-H08 [Major] [기능3(다음선호)] / [Mock검증]

| 필드 | 내용 |
|------|------|
| **Steps** | 목록 `{1,4,12,56}`, 현재 6 → `pressNextFavorite()` / Mock: `pressNumber(1)`+`pressConfirm()` |
| **Expected** | `"12"` / `setCH("1")` ×1 |
| **Actual** | `"6"` / `setCH` 0회 (`MockFunctionCalledWrongNumberOfTimes`) |
| **Root Cause** | `pressNextFavorite` 스텁; `pressConfirm` 미호출 |
| **Fix Summary** | `upper_bound` + wrap (`pressNextFavorite` L59–67); 확정 경로 `applyChannel` |

---

## FakeTuner

| ID | Severity | Area | 상태 |
|----|----------|------|------|
| — | — | FakeTuner | **결함 없음** |

`FakeTunerTest` 6건(경계 0/99, 예외 -1/100, `seekCH` 유효·wrap) 모두 Pass. README FakeTuner 요구와 일치.

---

## Green 확인 절차

```powershell
cmake --build build
ctest --test-dir build --output-on-failure

.\build\fake_tuner_test.exe
.\build\controller_test.exe
.\build\controller_mock_test.exe
```

| 명령 | 기대 |
|------|------|
| `ctest` (Windows) | 3/3 Passed |
| 직접 exe | 28/28 Passed |

---

## 후속

| 단계 | 내용 |
|------|------|
| Step 7 | `pressNumber` 상태 머신 리팩토링 (동작 변경 없음) |
| 커버리지 | README TO-DO — lcov 80% 목표 |
