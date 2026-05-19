# Step 5B — Defect list (Export Transcript)
_Exported on Tuesday, May 19, 2026 from Cursor_

---

## Turn 1

**User**

@step-5B_defect-list.md (1-10)

**과제 요약 (tasks/step-5B_defect-list.md)**

- [P] QA 리드
- [C] TDD TV Channel Controller — 현재까지 발견된 테스트 실패·결함 문서화
- [T] 발견된 결함을 다음 형식으로 정리
  - 항목: `[ID] [Severity] [Area] [Steps] [Expected] [Actual] [Root Cause] [Fix Summary]`
  - Area: FakeTuner / 기능1(숫자입력) / 기능2(선호토글) / 기능3(다음선호) / Mock검증 / Controller공통
  - ctest 실패 로그·step-5 분석 결과 반영; 없으면 README 시나리오 대비 구현 갭 기준
- [F] Markdown, `docs/defect_list.md`에도 저장

**참조 파일:** `README.md`, `tasks/step-1_requirements-analysis.md`, `tasks/step-5_debug-test-failures.md`, `Report/defect_list.md`, `docs/defect_list.md`, `include/TVChannelController.h`, `src/TVChannelController.cpp`, `include/FakeTuner.h`, `test/FakeTunerTest.cpp`, `test/TVChannelControllerTest.cpp`, `test/TVChannelControllerMockTest.cpp`

---

**Cursor**

Step 5B 결함 목록 작성을 수행했습니다. Step 5A 분석·소스·테스트·ctest 실행 결과를 반영했습니다.

**수행한 작업**

1. **컨텍스트 수집**
   - `Report/step-5A.debug_test_failures.md` — D-01(CTest 거짓 Green), D-02(문서 불일치), 스텁 시대 Red 패턴
   - `CMakeLists.txt` — Windows `add_test` 3건 적용 확인 (D-01 수정 완료)
   - `TVChannelController.cpp`, `FakeTuner.h`, 테스트 3종

2. **테스트 실행**
   - `ctest --output-on-failure` (build) → **3/3 Passed**
   - `fake_tuner_test.exe` → **6/6 PASSED**
   - `controller_test.exe` → **17/17 PASSED**
   - `controller_mock_test.exe` → **5/5 PASSED**

3. **결함 목록 작성**
   - **오픈 결함 0건** (프로덕션·FakeTuner·기능1~3·Mock)
   - **D-01** [Critical] [Controller공통] — 해결됨 (Windows CTest 한글 필터)
   - **D-02** [Info] [Controller공통] — Step 4 Report vs ctest 의미 불일치
   - **D-H01~H08** [Major] — 스텁 시대 회귀 참고 (Step 4에서 해소)
   - FakeTuner — 결함 없음

4. **산출물 저장**
   - `Report/defect_list.md`
   - `docs/defect_list.md`

**응답 요약 (Turn 1)**

| 구분 | 결과 |
|------|------|
| 현재 오픈 결함 | **0건** |
| ctest | **3/3 Passed** (exe 단위, 내부 28 GTest) |
| 직접 exe | **28/28 Passed** |
| D-01 | Critical · Controller공통 · **해결** (`CMakeLists.txt` L33–44) |
| D-02 | Info · Controller공통 · 문서/ctest 보고 불일치 |
| D-H01~H06 | Major · 기능1 · 스텁 Red (해결) |
| D-H07 | Major · 기능2 · 스텁 Red (해결) |
| D-H08 | Major · 기능3 / Mock검증 · 스텁 Red (해결) |
| FakeTuner | 결함 없음 (6/6) |

**결함 요약 테이블**

| ID | Severity | Area | 상태 | 요약 |
|----|----------|------|------|------|
| D-01 | Critical | Controller공통 | 해결 | Windows `gtest_discover_tests` + 한글 필터 → 0 tests ran, ctest Passed |
| D-02 | Info | Controller공통 | 정보 | ctest 28/28 vs 실제 검증 의미 불일치 |
| D-H01 | Major | 기능1(숫자입력) | 해결 | 스텁: 한 자리 확정 미적용 |
| D-H02 | Major | 기능1(숫자입력) | 해결 | 스텁: 두 자리 즉시 적용 미적용 |
| D-H03 | Major | 기능1(숫자입력) | 해결 | 스텁: 연속 두 자리 입력 미적용 |
| D-H04 | Major | 기능1(숫자입력) | 해결 | 스텁: Other 후 버퍼 무효화 미구현 |
| D-H05 | Major | 기능1(숫자입력) | 해결 | 스텁: 선행 0 (`0,7` → 7) 미적용 |
| D-H06 | Major | 기능1(숫자입력) | 해결 | 스텁: 채널 100+ 예외 미발생 |
| D-H07 | Major | 기능2(선호토글) | 해결 | 스텁: `pressFavorite` 빈 구현 |
| D-H08 | Major | 기능3(다음선호) / Mock검증 | 해결 | 스텁: `pressNextFavorite` / `setCH` 미호출 |

**D-01 상세 (해결됨)**

| 필드 | 내용 |
|------|------|
| **Steps** | `cmake --build build` 후 `ctest --test-dir build` (수정 전: per-case `gtest_discover_tests`) |
| **Expected** | 28개 GTest 각각 실행·assert |
| **Actual** | `LastTest.log`: `Running 0 tests` ×28, ctest Passed |
| **Root Cause** | 한글 `--gtest_filter` Windows 인코딩 불일치 |
| **Fix Summary** | Windows: `add_test` 3건 (실행 파일 단위); non-Windows: `gtest_discover_tests` 유지 |

**D-02 상세**

| 필드 | 내용 |
|------|------|
| **Steps** | Step 4 Report Green 근거 vs ctest 대조 |
| **Expected** | ctest 28/28 = 28 시나리오 검증 |
| **Actual** | D-01 시점 미실행인데 Passed; Green은 직접 exe와 일치 |
| **Fix Summary** | D-01 적용 후 ctest 3/3; Report에 Windows ctest 의미 명시 권장 |

**스텁 시대 회귀 참고 (D-H01~H08, 모두 Step 4 해소)**

| ID | Steps (요약) | Expected | Actual (스텁) |
|----|--------------|----------|---------------|
| D-H01 | `1` + Confirm | `"1"` | `"0"` |
| D-H02 | `1`,`2` | `"12"` | `"0"` |
| D-H03 | `1,2,3,4` | `"34"` | `"0"` |
| D-H04 | `4,5,6` + Other | `"45"` | `"0"` |
| D-H05 | `0`,`7` | `"7"` | `"0"` |
| D-H06 | `1`,`0`,`0` | `invalid_argument` | 예외 없음 |
| D-H07 | ch5 + Favorite | 목록 `{5}` | `{}` |
| D-H08 | next fav / Mock confirm | `"12"` / `setCH×1` | `"6"` / 0회 |

**Green 확인 절차**

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

## Turn 2

**User**

현재까지의 대화내용 전체를 Prompting 폴더에 Export Transcript 해 주세요. 파일명은 step- 으로 시작

---

**Cursor**

`Prompting/step-5B-defect-list-prompt.md`에 Export Transcript를 저장했습니다. (본 파일)

도구 호출·내부 로그·긴 소스 덤프는 제외하고, 과제 지시·수행 작업·산출물·응답 요약 위주로 읽기용 transcript로 정리했습니다.

---

## 부록: 주요 파일 경로

| 경로 | 설명 |
|------|------|
| `Report/defect_list.md` | Step 5B 결함 목록 Report |
| `docs/defect_list.md` | Step 5B 결함 목록 (docs 미러) |
| `Report/step-5A.debug_test_failures.md` | Step 5A 분석 (5B 근거) |
| `CMakeLists.txt` | D-01 수정 — Windows `add_test` 3건 (L33–44) |
| `src/TVChannelController.cpp` | 프로덕션 — 오픈 결함 없음 |
| `include/FakeTuner.h` | Test Double — 오픈 결함 없음 |
| `test/TVChannelControllerTest.cpp` | 상태 기반 17건 |
| `test/TVChannelControllerMockTest.cpp` | Mock 행위 5건 |
| `test/FakeTunerTest.cpp` | FakeTuner 6건 |
| `tasks/step-5B_defect-list.md` | 과제 지시문 |
| `tasks/step-5A_debug-test-failures.md` | 선행 Step 5A |
| `Prompting/step-5B-defect-list-prompt.md` | 본 Export Transcript |

## 부록: Step 5A → 5B 관계

| Step | 산출물 | 내용 |
|------|--------|------|
| 5A | `Report/step-5A.debug_test_failures.md` | ctest 거짓 Green 분석, CMake 수정, Green 재확인 |
| 5B | `Report/defect_list.md`, `docs/defect_list.md` | D-01·D-02·D-H01~H08 형식 정리, 오픈 0건 |

## 부록: Area별 분류

| Area | 결함 ID | 현재 상태 |
|------|---------|-----------|
| Controller공통 | D-01, D-02 | D-01 해결, D-02 정보 |
| FakeTuner | — | 결함 없음 |
| 기능1(숫자입력) | D-H01~D-H06 | 스텁 시대 해결 |
| 기능2(선호토글) | D-H07 | 스텁 시대 해결 |
| 기능3(다음선호) | D-H08 | 스텁 시대 해결 |
| Mock검증 | D-H08 | 스텁 시대 해결 |

## 부록: 프로덕션 구현 ↔ README (갭 없음)

| 시나리오 | 구현 근거 (`TVChannelController.cpp`) |
|----------|--------------------------------------|
| 두 자리 즉시 적용 | `10 < next ≤ 99` → `applyChannel` (L26–28) |
| `0,7` → 7 | `next < 10` → `applyChannel` (L21–23) |
| `1,0,0` → 예외 | `next==10` 보류 → 세 번째 자리 `>99` (L24–25, L33–35) |
| Other 후 6 미적용 | `pressOther` → `inputBuffer_=-1` (L69–71) |
| next favorite + wrap | `upper_bound` + `favorites_.front()` (L63–66) |
| 즐겨찾기 토글·정렬 | `pressFavorite` (L47–57) |

`ITuner.h` — **미수정**.

## 부록: Prompting · Report 시리즈

| 파일 | 단계 |
|------|------|
| `step-1-requirements-analysis-prompt.md` | Step 1 |
| `step-2-code-quality-prompt.md` | Step 2 |
| `step-3-test-plan-prompt.md` | Step 3 |
| `step-4-write-unit-tests-prompt.md` | Step 4 |
| `step-5A-debug-test-failures-prompt.md` | Step 5A |
| `step-5B-defect-list-prompt.md` | Step 5B (본 파일) |
| `Report/step-5A.debug_test_failures.md` | Step 5A Report |
| `Report/defect_list.md` | Step 5B Report |

## 부록: 후속 단계

| Step | 내용 |
|------|------|
| Step 6 | Golden Master |
| Step 7 | 리팩토링 (`clearBuffer`, `findNextFavorite`, lcov 80%) |
