# TDD TV Channel Controller — 결함 관리 보고서 (Step 8)

| 항목 | 내용 |
|------|------|
| 단계 | Step 8 — 결함 관리 문서 |
| 역할 | QA 리드 |
| 프로젝트 | TDD TV Channel Controller (C++17, Google Test / GMock, CMake, gcov/lcov) |
| 기준 문서 | `tasks/step-8_defect-report.md`, `docs/defect_list.md`, `docs/test_plan.md`, `Report/step-5B.defect_list.md`, `Report/step-6.golden_master.md` |
| 산출물 | `docs/defect_report.md`, `Report/step-8.defect_report.md`, `Prompting/step-8-defect-report-prompt.md` |
| 작성일 | 2026-05-19 |
| 선행 단계 | Step 5B 결함 목록, Step 6 Golden Master (ctest 4/4, 40 GTest) |
| 후속 단계 | Step 9 QA 최종 보고, Step 7 리팩토링 실행·lcov 80% |

---

## 1. 요약

Step 8에서는 **결함 분류 체계**, **보고서 템플릿**, **품질 메트릭 수집 계획**, **(선택) GitHub Issues 워크플로우**를 정의했다. 결함 상세 SSOT는 [`docs/defect_list.md`](../docs/defect_list.md)이며, 본 단계 산출물은 **프로세스·메트릭·매트릭스**를 담은 [`docs/defect_report.md`](../docs/defect_report.md)이다.

| 구분 | Step 5B (결함 목록) | Step 8 (본 단계) |
|------|---------------------|------------------|
| 목적 | 발견·해결 결함 ID·상세 기록 | 분류·보고·메트릭·이슈 연동 **체계** |
| 오픈 결함 | 0건 | 0건 (동일) |
| 테스트 기준선 | 28 GTest (5B 시점) | **40 GTest** (Golden 12 포함) |
| ctest | 3/4 executable (5B) | **4/4** (`golden_test` 포함) |

**품질 스냅샷 (2026-05-19):**

| 메트릭 | 값 |
|--------|-----|
| 오픈 결함 | **0** |
| `fake_tuner_test` | 6/6 |
| `controller_test` | 17/17 |
| `controller_mock_test` | 5/5 |
| `golden_test` | 12/12 |
| **GTest 합계** | **40/40 (100%)** |
| **ctest (Windows)** | **4/4 (100%)** |
| lcov (`tv_controller`) | **미측정** (Step 9·리팩토링 게이트) |

---

## 2. 과제 요구사항 대응

| # | 요구 (Step 8) | 산출 | 근거 |
|---|---------------|------|------|
| 1 | Severity × Area 매트릭스 (5 Area) | §1.3 오픈 0 / 누적 히스토리 표 | `docs/defect_report.md` §1 |
| 2 | 결함 보고서 템플릿 (재현/기대/실제/원인/수정/검증) | §2 표준 템플릿 + D-01·D-H06 예시 | `defect_list.md` ID 연계 |
| 3 | 품질 메트릭 (4 test exe, lcov 80%+, 단계별 발견율) | §3 통과율·커버리지·step-4~7 표 | README·test_plan |
| 4 | (선택) GitHub Issues 라벨·PR·ctest 연계 | §4 워크플로·라벨·mermaid | 선택 구현 |
| 5 | `docs/defect_report.md` 저장 | **완료** | 실행 문서 |
| 6 | Report·Prompting·GitHub 업로드 | **완료** (본 Report·transcript·push) | 사용자 요청 Turn 2 |

---

## 3. 결함 분류 체계 (요약)

### 3.1 Severity

| Severity | 정의 | 프로젝트 예 |
|----------|------|-------------|
| Critical | 테스트 게이트 붕괴·전면 불가 | D-01 Windows CTest 거짓 Pass |
| Major | README/플랜 시나리오 불일치 | D-H01~H08 (스텁, 해결) |
| Minor | 비핵심·우회 가능 | (미등록) |
| Info | 문서·메트릭 해석 | D-02 ctest vs 40 GTest 의미 |

### 3.2 Area (5종)

| Area | 테스트 자산 | GTest 수 |
|------|-------------|:--------:|
| FakeTuner | `fake_tuner_test` | 6 |
| 기능1(숫자입력) | `controller_test` + Golden 채널 | (분산) |
| 기능2(선호토글) | `controller_test` + Golden 선호 | (분산) |
| 기능3(다음선호) | `controller_test` + Golden next | (분산) |
| Mock검증 | `controller_mock_test` | 5 |

**전역(인프라):** D-01, D-02 — 5 Area 외 별도 집계.

### 3.3 Severity × Area — 오픈 결함

|  | FakeTuner | 기능1 | 기능2 | 기능3 | Mock | 합계 |
|--|:---------:|:-----:|:-----:|:-----:|:----:|:----:|
| Critical~Info | 0 | 0 | 0 | 0 | 0 | **0** |

### 3.4 누적 (회귀 참고)

|  | FakeTuner | 기능1 | 기능2 | 기능3 | Mock |
|--|:---------:|:-----:|:-----:|:-----:|:----:|
| Major (해결) | 0 | 6 | 1 | 1† | 1† |

† D-H08: 기능3 + Mock 동시 실패 (ID 1건).  
전역: D-01 Critical 해결, D-02 Info.

상세 ID 매핑: [`docs/defect_list.md`](../docs/defect_list.md) 요약 테이블.

---

## 4. 결함 보고서 템플릿

모든 신규 결함은 아래 필드를 `defect_list.md`에 기록한다.

| 필드 | 설명 |
|------|------|
| 재현 (Steps) | GTest/Golden 재현 순서 |
| 기대 (Expected) | README·플랜 기준 |
| 실제 (Actual) | 실패 로그·assert |
| 원인 (Root Cause) | 코드·인프라 근본 원인 |
| 수정 (Fix) | 파일·커밋 요약 |
| 검증 (Verification) | 40/40, ctest 4/4, lcov |

**대표 카드 (발췌):**

- **D-01 [Critical][전역]:** 한글 `gtest_filter` → CTest 0 tests Pass → `CMakeLists.txt` WIN32 executable 4건 등록.
- **D-H06 [Major][기능1]:** `1,0,0` → `invalid_argument` 미발생 → 범위·세 자리 검사 추가.

전체 템플릿·예시: [`docs/defect_report.md`](../docs/defect_report.md) §2.

---

## 5. 품질 메트릭 수집 계획

### 5.1 테스트 통과율

```
통과율(전체) = Σ Passed / 40 × 100%
ctest (Windows) = 4/4 executable (내부 40 GTest는 직접 exe로 SSOT — D-02)
```

| Executable | 건수 | Area |
|------------|:----:|------|
| `fake_tuner_test` | 6 | FakeTuner |
| `controller_test` | 17 | 기능1~3 |
| `controller_mock_test` | 5 | Mock |
| `golden_test` | 12 | E2E |
| **합계** | **40** | — |

**표준 검증:**

```powershell
cmake --build build
ctest --test-dir build --output-on-failure
.\build\fake_tuner_test.exe
.\build\controller_test.exe
.\build\controller_mock_test.exe
.\build\golden_test.exe
```

### 5.2 커버리지

| 항목 | 내용 |
|------|------|
| 대상 | `tv_controller` → `TVChannelController.cpp` |
| 목표 | README **≥ 80%** (권장 90%) |
| 도구 | gcov + lcov (`ENABLE_COVERAGE`) |
| 현재 | 미측정 → Step 7 Phase 10 / Step 9 |

### 5.3 단계별 결함 발견율

| 단계 | 발견 (고유 ID) | 해결 | 오픈 |
|------|:--------------:|:----:|:----:|
| step-4 | 8 (D-H01~H08) | 8 | 0 |
| step-5 | 2 (D-01, D-02) | 1 | 0 (D-02 Info) |
| step-6 | 0 | — | 0 |
| step-7 | 0 (현재) | — | 0 |

step-5 이후 **Major+Critical 신규 0** — 리팩토링·QA 종료 전 품질 안정 구간.

---

## 6. GitHub Issues 연동 (선택)

| 구분 | 라벨 예 |
|------|---------|
| Severity | `severity:critical` … `severity:info` |
| Area | `area:fake-tuner`, `area:feature-1-input`, … `area:infra` |
| 기타 | `defect`, `regression`, `golden-master` |

- SSOT: `docs/defect_list.md` → Issue 미러
- PR: `Fixes #NNN`, 체크리스트 40/40 + lcov
- ctest 실패 시 `area:infra` + 로그 링크 (D-01 재발 방지)

상세: [`docs/defect_report.md`](../docs/defect_report.md) §4.

---

## 7. 산출물·문서 맵

| 파일 | 용도 |
|------|------|
| `docs/defect_list.md` | 결함 SSOT (ID·상세) |
| `docs/defect_report.md` | 분류·템플릿·메트릭·Issues (실행 문서) |
| `Report/step-8.defect_report.md` | 본 Report (요약·대응표) |
| `Prompting/step-8-defect-report-prompt.md` | Cursor 대화 Export Transcript |
| `tasks/step-8_defect-report.md` | 과제 지시문 |

---

## 8. 후속 단계

| Step | 내용 |
|------|------|
| Step 7 구현 | `dev/refactoring` Phase 1~10, 40/40·Golden 유지 |
| Step 9 | QA 최종 — 통과율·lcov 수치·결함 종료 보고 |
| lcov | `tv_controller` **80%+** 달성 후 README 체크리스트 완료 |

Step 8은 **결함 0건 오픈** 상태에서 **측정·보고 프로세스**를 확립했다. 이후 리팩토링·QA는 본 문서의 게이트(40/40, 4/4 ctest, lcov)로 회귀를 관리한다.
