# TDD TV Channel Controller — Golden Master 회귀 테스트

| 항목 | 내용 |
|------|------|
| 단계 | Step 6 — Golden Master (Approval) 회귀 테스트 |
| 역할 | 회귀 테스트(Approval / Golden Master) 설계 |
| 프로젝트 | TDD TV Channel Controller (C++17, CMake, Google Test / GMock) |
| 기준 문서 | `README.md` TO-DO, `Report/step-3.test_plan.md` (T-01~T-28), `Report/step-4.write_unit_tests.md` |
| 산출물 | `test/TVChannelGoldenTest.cpp`, `test/golden_master/ScenarioRunner.h`, `test/golden/*`, `CMakeLists.txt`, `.github/workflows/ci.yml`, `docs/golden_master_guide.md` |
| 검증 명령 | `cmake --build build && ctest --test-dir build --output-on-failure` |
| 작성일 | 2026-05-19 |
| 선행 단계 | Step 4 Green (28 단위 테스트), Step 5A/5B ctest·결함 정리 |
| 후속 단계 | Step 7 리팩토링 (Golden을 회귀 기준선으로 유지) |
| 제약 | `ITuner.h` **수정 없음** — `FakeTuner`만 사용 |

---

## 1. 요약

README 시나리오를 **텍스트 스냅샷**으로 고정하는 Golden Master 회귀 테스트를 설계·구현했다. 시나리오 스크립트(`.scenario`)를 실행한 뒤 단계별 `CH=<채널> FAV=[...]` 덤프 전체를 기대 파일(`.approved.txt`)과 비교한다.

| 구분 | Step 4 (단위) | Step 6 (Golden Master) |
|------|---------------|------------------------|
| 검증 방식 | `EXPECT_EQ` 등 개별 assertion | **전체 출력 문자열** 파일 비교 |
| 테스트 수 | 28건 (3 executable) | **12 시나리오** (`golden_test` 1 executable) |
| Test Double | Fake + Mock | **FakeTuner만** |
| 산출 위치 | `test/*Test.cpp` | `test/golden/*.scenario` + `*.approved.txt` |
| ctest | 3 + 1 = **4 executable** | `golden_test` 추가 |
| CI | — | GitHub Actions `ci.yml` |

**검증 결과 (로컬, 2026-05-19):**

```text
ctest --test-dir build --output-on-failure

100% tests passed, 0 tests failed out of 4
  fake_tuner_test, controller_test, controller_mock_test, golden_test
```

`golden_test` 직접 실행: **12/12 Passed**.

---

## 2. 과제 요구사항 대응

| # | 요구 (Step 6) | 구현 | 근거 |
|---|---------------|------|------|
| 1 | 시나리오 → 스냅샷 형식 정의 | `CH=… FAV=[…]` 한 줄/단계, 예외 시 `!EXCEPTION` | `ScenarioRunner.h`, §3 |
| 2 | expected 파일 보관·갱신 | `test/golden/<name>.approved.txt`, `UPDATE_GOLDENS=1` | `docs/golden_master_guide.md` §3·§5 |
| 3 | GTest 파일 비교 | `TVChannelGoldenTest.cpp`, `EXPECT_EQ(expected, actual)` | §4 |
| 4 | CMake / ctest | `golden_test` 타깃, `GOLDEN_DIR` 정의, ctest 등록 | `CMakeLists.txt` |
| 5 | CI 자동 실행 | `.github/workflows/ci.yml` (ubuntu, ctest) | §8 |
| — | `ITuner.h` 수정 금지 | **미수정** | `include/ITuner.h` 변경 없음 |

---

## 3. 스냅샷 출력 형식

각 시나리오는 **초기 상태 1줄** + **액션마다 1줄**을 출력한다.

```text
CH=<현재채널> FAV=[<오름차순 즐겨찾기, 쉼표 구분>]
```

| 예시 | 의미 |
|------|------|
| `CH=0 FAV=[]` | 채널 0, 즐겨찾기 없음 |
| `CH=12 FAV=[6,12,37]` | 채널 12, 즐겨찾기 6·12·37 |
| `!EXCEPTION 채널 범위 초과: 100` | 마지막 액션에서 `std::invalid_argument` |

채널 문자열은 `FakeTuner::getCurrentCH()` 반환값(선행 0 없음, `"7"` 형태)을 그대로 사용한다.

---

## 4. 시나리오 스크립트 DSL

파일: `test/golden/<name>.scenario` — 한 줄에 하나의 액션, `#` 주석·빈 줄 무시.

| 토큰 | API |
|------|-----|
| `N0`…`N9`, `N <d>` | `pressNumber(d)` |
| `C` | `pressConfirm()` |
| `F` | `pressFavorite()` |
| `NF` | `pressNextFavorite()` |
| `O` | `pressOther()` |
| `T <ch>` | 채널 이동 헬퍼 (한/두 자리 입력) |
| `@CH <ch>` | `tuner.setCH(...)` |
| `@FAV 1,4,12` | `addFavorite` 반복 |

### 샘플: `channel_single_confirm`

**입력 (`.scenario`):**

```text
N1
C
```

**기대 (`.approved.txt`):**

```text
CH=0 FAV=[]
CH=0 FAV=[]
CH=1 FAV=[]
```

첫 `N1`은 버퍼만 설정하고 채널은 0 유지 → `pressConfirm()` 후 채널 1.

---

## 5. 아키텍처

```text
TVChannelGoldenTest.cpp
    └─ GoldenMasterTest (FakeTuner + TVChannelController)
           └─ ScenarioRunner.h
                  ├─ loadScenario(.scenario)
                  ├─ runScenario() → trace string
                  └─ readFile(.approved.txt) / writeFile (UPDATE_GOLDENS=1)

CMake: golden_test
    └─ GOLDEN_DIR = ${CMAKE_SOURCE_DIR}/test/golden
```

| 파일 | 역할 |
|------|------|
| `test/golden_master/ScenarioRunner.h` | DSL 파싱, 상태 덤프, 파일 I/O, 갱신 플래그 |
| `test/TVChannelGoldenTest.cpp` | 시나리오별 `TEST_F`, golden 비교 |
| `test/golden/*.scenario` | 12개 입력 스크립트 |
| `test/golden/*.approved.txt` | 12개 기대 스냅샷 (소스 트리 커밋) |
| `docs/golden_master_guide.md` | 실행·갱신·CI 운영 가이드 |

---

## 6. 시나리오 목록 및 README 추적

| Golden 이름 | T-ID / README | 검증 포인트 |
|-------------|---------------|-------------|
| `channel_single_confirm` | T-01 | 한 자리 + 확정 → CH=1 |
| `channel_two_digit_immediate` | T-02 | `N1` `N2` → 즉시 CH=12 |
| `channel_sequence_12_then_34` | T-03 | 12 후 34 연속 변경 |
| `channel_buffer_cleared_by_other` | T-04 | 45 적용 후 버퍼 6, `O` → CH=45 유지 |
| `channel_leading_zero_07` | T-05 | `N0` `N7` → CH=7 |
| `channel_99_boundary` | T-07 | CH=99 |
| `favorites_complex_toggle` | T-12 | 12→★ … → FAV=[6,12,37] |
| `next_favorite_from_6` | T-16 | 현재 6, 목록 {1,4,12,56} → CH=12 |
| `next_favorite_wrap_56_to_1` | T-17 | 56 → wrap CH=1 |
| `next_favorite_empty_list` | T-18 | 빈 목록, CH=6 유지 |
| `exception_channel_100` | T-08 | `N1` `N0` `N0` → `!EXCEPTION` |
| `readme_smoke_end_to_end` | 통합 | 입력·즐겨찾기·다음 즐겨찾기 연속 |

단위 테스트(Step 4)가 **메서드·경계·Mock 호출**을 검증한다면, Golden Master는 **사용자 조작 시퀀스 전체의 최종 상태 궤적**을 고정한다. T-06, T-09~T-11, T-13~T-15, T-19~T-28 등은 기존 `*_test.cpp`에 남기고, 대표·통합 시나리오만 Golden에 선별했다.

---

## 7. CMake / ctest 통합

`CMakeLists.txt` 변경 요약:

```cmake
add_executable(golden_test test/TVChannelGoldenTest.cpp)
target_link_libraries(golden_test tv_controller GTest::gtest_main)
target_compile_definitions(golden_test PRIVATE
    GOLDEN_DIR="${CMAKE_SOURCE_DIR}/test/golden"
)

# Windows: add_test(NAME golden_test COMMAND golden_test)
# Linux:   gtest_discover_tests(golden_test)
```

| 플랫폼 | ctest 등록 | 비고 |
|--------|------------|------|
| Windows | executable 단위 `golden_test` | Step 5A와 동일 — 한글 필터 깨짐 회피 |
| Linux/macOS | `gtest_discover_tests(golden_test)` | 케이스별 ctest 항목 생성 |

---

## 8. Golden 갱신 절차

의도적 동작 변경 시에만 사용한다.

**Windows (PowerShell):**

```powershell
$env:UPDATE_GOLDENS = "1"
.\build\golden_test.exe
Remove-Item Env:UPDATE_GOLDENS
.\build\golden_test.exe   # 12/12 Passed 확인
```

**Linux / macOS:**

```bash
UPDATE_GOLDENS=1 ./build/golden_test
./build/golden_test
```

`UPDATE_GOLDENS=1` 실행 시 `.approved.txt` 덮어쓰기 후 해당 테스트는 **Skipped**; 이후 일반 실행으로 Pass 확인 → diff 리뷰 → 커밋.

상세: [`docs/golden_master_guide.md`](../docs/golden_master_guide.md).

---

## 9. CI 구성

파일: [`.github/workflows/ci.yml`](../.github/workflows/ci.yml)

| 항목 | 값 |
|------|-----|
| 트리거 | `push` / `pull_request` → `main`, `dev` |
| Runner | `ubuntu-latest` |
| 빌드 | `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release` |
| 테스트 | `ctest --test-dir build --output-on-failure` (golden_test 포함) |

로컬에서 CI와 동일 검증:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 10. 단위 테스트 vs Golden Master

| 관점 | 단위 테스트 (`*_test.cpp`) | Golden Master (`golden_test`) |
|------|---------------------------|------------------------------|
| 목적 | TDD Red→Green, API·경계·Mock 계약 | **회귀 방어**, E2E 상태 궤적 |
| 실패 진단 | assertion 메시지·라인 | 시나리오 전체 diff |
| 유지보수 | 시나리오 추가 시 테스트 코드 | `.scenario` + `.approved.txt` + `TEST_F` 1줄 |
| 변경 시 | 테스트/코드 수정 | `UPDATE_GOLDENS` 또는 수동 `.approved.txt` |

Step 7 리팩토링 시: 단위 테스트 + Golden Master를 함께 실행해 동작 동일성을 확인한다.

---

## 11. 제약 준수 및 리스크

| 항목 | 상태 |
|------|------|
| `ITuner.h` 미수정 | ✅ |
| `FakeTuner`만 사용 | ✅ |
| Windows ctest 한글 필터 | `golden_test`는 executable 단위 등록 → **12건 실제 실행** |
| Golden 무분별 갱신 | 가이드에 diff 리뷰·의도적 변경만 갱신 명시 |
| 스냅샷 메시지 로캘 | 예외 메시지 `채널 범위 초과: 100` — 구현 문자열에 의존 |

---

## 12. 결론

- README 기반 **12개 Golden Master 시나리오**와 **24개 golden 파일**(scenario + approved)을 추가했다.
- `golden_test` executable 및 ctest·CI에 통합해 Step 4 이후 **회귀 기준선**을 확보했다.
- 운영 문서는 [`docs/golden_master_guide.md`](../docs/golden_master_guide.md)에, 본 보고서는 Step 6 산출 요약이다.

**한 줄 요약:** 단위 테스트 28건 위에, 사용자 시나리오 스냅샷 12건으로 **Approval 스타일 회귀 방어막**을 구축했고, 로컬·CI 모두 Green이다.
