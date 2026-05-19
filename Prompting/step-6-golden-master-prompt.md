# Step 6 — Golden Master (Export Transcript)
_Exported on Tuesday, May 19, 2026 from Cursor_

---

## Turn 1

**User**

@step-6_golden-master.md (1-20)

**과제 요약 (tasks/step-6_golden-master.md)**

- [P] 회귀 테스트(Approval / Golden Master) 설계 전문가
- [C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
- [T] README 시나리오 기반 Golden Master 회귀 테스트 설계·구현
  1. 시나리오 스크립트 → 텍스트 스냅샷 출력 형식 정의 (`pressNumber`/`pressConfirm`/`pressFavorite`/`pressNextFavorite`/`pressOther` 시퀀스, 각 단계 후 `CH=<현재채널> FAV=[...]` 한 줄 덤프)
  2. expected 출력 파일 생성·보관 전략 (`test/golden/`, 시나리오별 `.approved.txt`, 갱신 절차 문서화)
  3. Google Test에서 파일 비교 테스트 (`TVChannelGoldenTest.cpp`, FakeTuner, 전체 출력 vs golden 파일)
  4. CMake / ctest 통합 (`golden_test`, `gtest_discover_tests` 포함)
  5. CI 자동 실행 (GitHub Actions 또는 로컬 `ctest` 설명)
  - **제약:** `ITuner.h` 수정 금지
- [F] 테스트 코드 + CMake 수정안 + golden 파일 샘플 + 실행·갱신 방법 (Markdown)
  - `docs/golden_master_guide.md`에도 저장

**참조 파일:** `README.md`, `tasks/step-1_requirements-analysis.md`, `include/TVChannelController.h`, `src/TVChannelController.cpp`, `include/FakeTuner.h`, `test/TVChannelControllerTest.cpp`, `CMakeLists.txt`

---

**Cursor**

Step 6 Golden Master 회귀 테스트를 설계·구현했습니다. 코드베이스·README·기존 단위 테스트를 확인한 뒤 프레임워크·시나리오·Golden 파일·CMake·CI·가이드를 추가했습니다.

**수행한 작업**

1. **스냅샷 형식 정의**
   - 각 단계 직후: `CH=<현재채널> FAV=[<오름차순, 쉼표 구분>]`
   - 초기 상태 1줄 + 액션마다 1줄
   - 예외 시나리오: 마지막 줄 `!EXCEPTION <메시지>`

2. **시나리오 DSL (`.scenario`)**
   - `N0`…`N9`, `N <d>`, `C`, `F`, `NF`, `O`
   - 헬퍼: `T <ch>` (채널 이동), `@CH <ch>`, `@FAV 1,4,12`
   - 파서·실행기: `test/golden_master/ScenarioRunner.h`

3. **Google Test 파일 비교**
   - `test/TVChannelGoldenTest.cpp` — `GoldenMasterTest` 픽스처, 12 시나리오
   - `EXPECT_EQ(expected, actual)` 전체 문자열 비교
   - `UPDATE_GOLDENS=1` 시 `.approved.txt` 갱신 후 `GTEST_SKIP`

4. **Golden 파일 (12쌍)**
   - `test/golden/<name>.scenario` + `<name>.approved.txt`
   - `UPDATE_GOLDENS=1`로 생성 후 일반 실행으로 검증

5. **CMake / ctest**
   - `add_executable(golden_test …)`
   - `GOLDEN_DIR="${CMAKE_SOURCE_DIR}/test/golden"`
   - Windows: `add_test(NAME golden_test …)` (한글 필터 회피)
   - Linux: `gtest_discover_tests(golden_test)`

6. **CI**
   - `.github/workflows/ci.yml` — ubuntu, Ninja, `ctest --output-on-failure`

7. **문서**
   - `docs/golden_master_guide.md` — DSL, 실행, 갱신, CI, 디버깅

8. **빌드·검증**
   - `cmake --build build` — `formatState` const 수정 (`FakeTuner::getCurrentCH` 비 const)
   - `UPDATE_GOLDENS=1` → 12개 `.approved.txt` 생성
   - `golden_test.exe` → **12/12 Passed**
   - `ctest` → **4/4 Passed** (fake_tuner, controller, controller_mock, golden)

**응답 요약 (Turn 1)**

| 구분 | 결과 |
|------|------|
| Golden 시나리오 | **12건** |
| ctest executable | **4/4 Passed** |
| `golden_test` GTest | **12/12 Passed** |
| `ITuner.h` | **미수정** |
| Test Double | **FakeTuner만** |

**스냅샷 예시 (`channel_single_confirm`)**

```text
# .scenario
N1
C

# .approved.txt
CH=0 FAV=[]
CH=0 FAV=[]
CH=1 FAV=[]
```

**포함 시나리오 (README / T-ID)**

| Golden 이름 | T-ID |
|-------------|------|
| `channel_single_confirm` | T-01 |
| `channel_two_digit_immediate` | T-02 |
| `channel_sequence_12_then_34` | T-03 |
| `channel_buffer_cleared_by_other` | T-04 |
| `channel_leading_zero_07` | T-05 |
| `channel_99_boundary` | T-07 |
| `favorites_complex_toggle` | T-12 |
| `next_favorite_from_6` | T-16 |
| `next_favorite_wrap_56_to_1` | T-17 |
| `next_favorite_empty_list` | T-18 |
| `exception_channel_100` | T-08 |
| `readme_smoke_end_to_end` | 통합 스모크 |

**CMake 추가 요약**

```cmake
add_executable(golden_test test/TVChannelGoldenTest.cpp)
target_link_libraries(golden_test tv_controller GTest::gtest_main)
target_compile_definitions(golden_test PRIVATE
    GOLDEN_DIR="${CMAKE_SOURCE_DIR}/test/golden"
)
# Windows: add_test(NAME golden_test COMMAND golden_test)
# else:    gtest_discover_tests(golden_test)
```

**Golden 갱신 (PowerShell)**

```powershell
$env:UPDATE_GOLDENS = "1"
.\build\golden_test.exe
Remove-Item Env:UPDATE_GOLDENS
.\build\golden_test.exe
```

**신규·수정 파일**

| 경로 | 설명 |
|------|------|
| `test/golden_master/ScenarioRunner.h` | DSL 파싱, trace 생성, 파일 I/O |
| `test/TVChannelGoldenTest.cpp` | 12× `TEST_F` |
| `test/golden/*.scenario` | 입력 스크립트 12개 |
| `test/golden/*.approved.txt` | 기대 스냅샷 12개 |
| `CMakeLists.txt` | `golden_test` + ctest |
| `.github/workflows/ci.yml` | CI |
| `docs/golden_master_guide.md` | 운영 가이드 |

---

## Turn 2

**User**

Report 폴더에 보고서를 작성해서 저장해 주세요. 파일명은 step- 으로 시작

---

**Cursor**

`Report/step-6.golden_master.md`에 Step 6 보고서를 저장했습니다. 기존 `step-4`, `step-5A` 보고서 형식을 따랐습니다.

**Report 구성**

- 메타데이터(단계, 산출물, 검증 명령, 선·후속 단계, `ITuner.h` 미수정)
- 요약 및 Step 4 대비 비교
- 과제 5항목 대응 표
- 스냅샷 형식·DSL·아키텍처
- 12개 시나리오 README/T-ID 매핑
- CMake/ctest·Golden 갱신·CI
- 단위 테스트 vs Golden Master, 제약·리스크, 결론

**산출물:** [`Report/step-6.golden_master.md`](../Report/step-6.golden_master.md)

---

## Turn 3

**User**

현재까지의 대화내용 전체를 Prompting 폴더에 Export Transcript 해 주세요. 파일명은 step- 으로 시작

---

**Cursor**

`Prompting/step-6-golden-master-prompt.md`에 Export Transcript를 저장했습니다. (본 파일)

도구 호출·내부 로그·긴 소스 덤프는 제외하고, 과제 지시·수행 작업·산출물·응답 요약 위주로 읽기용 transcript로 정리했습니다.

---

## 부록: 주요 파일 경로

| 경로 | 설명 |
|------|------|
| `test/golden_master/ScenarioRunner.h` | 시나리오 DSL·스냅샷 실행기 |
| `test/TVChannelGoldenTest.cpp` | Golden Master GTest (12건) |
| `test/golden/*.scenario` | 시나리오 입력 스크립트 |
| `test/golden/*.approved.txt` | 기대 스냅샷 (커밋 대상) |
| `CMakeLists.txt` | `golden_test`, `GOLDEN_DIR`, ctest |
| `.github/workflows/ci.yml` | GitHub Actions CI |
| `docs/golden_master_guide.md` | 실행·갱신·CI 가이드 |
| `Report/step-6.golden_master.md` | Step 6 Report |
| `tasks/step-6_golden-master.md` | 과제 지시문 |
| `Prompting/step-6-golden-master-prompt.md` | 본 Export Transcript |

## 부록: 시나리오 DSL 토큰

| 토큰 | API |
|------|-----|
| `N0`…`N9`, `N <d>` | `pressNumber(d)` |
| `C` | `pressConfirm()` |
| `F` | `pressFavorite()` |
| `NF` | `pressNextFavorite()` |
| `O` | `pressOther()` |
| `T <ch>` | 채널 이동 헬퍼 |
| `@CH <ch>` | `tuner.setCH(...)` |
| `@FAV 1,4,12` | `addFavorite` 반복 |

## 부록: 검증 명령

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
.\build\golden_test.exe
```

| 명령 | 기대 |
|------|------|
| `ctest` (Windows) | 4/4 Passed |
| `golden_test.exe` | 12/12 Passed |
| 직접 exe 3종 (Step 4) | 28/28 Passed |

## 부록: 단위 테스트 vs Golden Master

| 관점 | 단위 테스트 | Golden Master |
|------|-------------|---------------|
| 파일 | `*_test.cpp` (3종) | `golden_test` + `test/golden/` |
| 검증 | 개별 `EXPECT_*` | 전체 trace 문자열 |
| Test Double | Fake + Mock | Fake만 |
| 목적 | TDD Green, API·Mock 계약 | E2E 상태 궤적 회귀 방어 |

## 부록: Prompting · Report 시리즈

| 파일 | 단계 |
|------|------|
| `step-1-requirements-analysis-prompt.md` | Step 1 |
| `step-2-code-quality-prompt.md` | Step 2 |
| `step-3-test-plan-prompt.md` | Step 3 |
| `step-4-write-unit-tests-prompt.md` | Step 4 |
| `step-5A-debug-test-failures-prompt.md` | Step 5A |
| `step-5B-defect-list-prompt.md` | Step 5B |
| `step-6-golden-master-prompt.md` | Step 6 (본 파일) |
| `Report/step-6.golden_master.md` | Step 6 Report |

## 부록: 후속 단계

| Step | 내용 |
|------|------|
| Step 7 | 리팩토링 (`clearBuffer`, `findNextFavorite`, lcov 80%) — Golden·단위 테스트를 회귀 기준선으로 유지 |
| Step 8~9 | 결함 보고·QA 최종 |

`ITuner.h` — **미수정**.
