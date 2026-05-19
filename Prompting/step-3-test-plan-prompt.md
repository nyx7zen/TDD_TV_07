# Step 3 — Test plan (Export Transcript)
_Exported on Tuesday, May 19, 2026 from Cursor_

---

## Turn 1

**User**

@step-3_test-plan.md (1-17)

**과제 요약 (tasks/step-3_test-plan.md)**

- [P] 시니어 QA 리드
- [C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake, gcov/lcov 가능 시 포함)
- [T] 테스트 계획서 작성
  - TEST / TEST_F 기반 단위 테스트 범위·우선순위 (`fake_tuner_test` → `controller_test` → `controller_mock_test`, TDD Red→Green)
  - 경계값 케이스 목록 (채널 0, 1, 98, 99 / -1, 100 / digit 0~9 / `inputBuffer_` -1 / 빈·단일 `favorites_` / wrap-around)
  - 예외·특이 케이스 목록 (`std::invalid_argument`, `pressOther` 버퍼 무효화, 목록 외 현재 채널, Mock 미호출 등)
  - Fake vs Mock 검증 분리 전략 (상태 기반 vs 행위 기반, `EXPECT_CALL`·`InSequence`)
  - 커버리지 목표 (README 80%+, 권장 90%+)와 gcov/lcov 측정·개선 전략
  - README TO-DO 28개 시나리오 추적 매트릭스 (번호·테스트명·우선순위)
- [F] Markdown, `docs/test_plan.md`에 저장

**참조 파일:** `README.md`, `tasks/step-1_requirements-analysis.md`, `include/TVChannelController.h`, `src/TVChannelController.cpp`, `include/FakeTuner.h`, `include/MockTuner.h`, `test/FakeTunerTest.cpp`, `test/TVChannelControllerTest.cpp`, `test/TVChannelControllerMockTest.cpp`, `CMakeLists.txt`

---

**Cursor**

테스트 계획서 작성을 위해 README·요구사항·소스·테스트·CMake를 읽고 `docs/test_plan.md`를 작성했습니다.

**수행한 작업**

1. **소스·명세 읽기**
   - `README.md` — TO-DO 테스트 28건 (Fake 6 + Controller 17 + Mock 5)
   - `docs/requirements_analysis.md` / `Report/step-1.requirements_analysis.md` — F1~F3, FT, MK, T-01~T-28
   - `include/TVChannelController.h` — `inputBuffer_ = -1`, `favorites_`, `isValidChannel`, `applyChannel` (private)
   - `src/TVChannelController.cpp` — **전 public 메서드·`applyChannel` 스텁**
   - `include/FakeTuner.h` — `setCH` 0~99, `seekCH` wrap 구현 완료
   - `include/MockTuner.h` — `MOCK_METHOD` 3개
   - 테스트 3종 — 각 1개 스텁 (`Stub`)만 존재
   - `CMakeLists.txt` — `fake_tuner_test`, `controller_test`, `controller_mock_test` 등록 확인

2. **테스트 계획서 작성**
   - `docs/test_plan.md` 신규 작성 (섹션 1~8)

**산출물 구조 요약**

| 섹션 | 내용 |
|------|------|
| 1 | 단위 테스트 범위·우선순위 — Phase A~F, `TEST`/`TEST_F`, P0/P1/P2 |
| 2 | 경계값 — 채널 0/1/98/99, -1/100, digit 0~9, `inputBuffer_`, favorites, seek wrap |
| 3 | 예외·특이 — `invalid_argument`, T-10 버퍼 무효화, Mock 미호출·순서 |
| 4 | Fake vs Mock — 상태 vs 행위, `EXPECT_CALL`/`InSequence` 예시 코드 |
| 5 | 커버리지 80%+/90%+, CMake `ENABLE_COVERAGE`, lcov 파이프라인 |
| 6 | README TO-DO 28건 추적 매트릭스 (T-01~T-28, 권장 테스트명) |
| 7~8 | 실행·DoD, 현재 스텁 상태 |

**핵심 계획 요약 (응답 요약)**

| 항목 | 내용 |
|------|------|
| **진행 순서** | `fake_tuner_test` (T-01~06) → `controller_test` (T-07~23) → `controller_mock_test` (T-24~28) → lcov Phase F |
| **P0** | T-01~04, T-07~08, T-14 (경계·두 자리·예외) |
| **Fake** | `getCurrentCH()`, `getFavoriteChannels()`, `EXPECT_THROW` |
| **Mock** | `setCH` 횟수, `getCurrentCH`→`setCH` `InSequence`, 빈 목록 `Times(0)` |
| **현재 상태** | 테스트 3건 스텁, 컨트롤러 스텁 — Phase A Red 착수 대기 |

---

## Turn 2

**User**

Report 폴더에 보고서를 작성해서 저장해 주세요. 파일명은 step- 으로 시작

---

**Cursor**

`Report/step-3.test_plan.md`에 Step 3 테스트 계획 보고서를 저장했습니다. (`step-1.requirements_analysis.md`, `step-2.code_quality.md`와 동일한 `step-{번호}.{주제}.md` 형식)

**수행한 작업**

- `docs/test_plan.md` 내용을 Report 형식 메타데이터와 함께 `Report/step-3.test_plan.md`로 정리
- 메타데이터: 단계 Step 3, 기준 `Report/step-1.requirements_analysis.md`, 동기 문서 `docs/test_plan.md`, 작성일 2026-05-19

---

## Turn 3

**User**

현재까지의 대화내용 전체를 Prompting 폴더에 Export Transcript 해 주세요. 파일명은 step- 으로 시작

---

**Cursor**

`Prompting/step-3-test-plan-prompt.md`에 Export Transcript를 저장했습니다. (본 파일)

도구 호출·내부 로그는 제외하고, 과제 지시·수행 작업·산출물·응답 요약 위주로 읽기용 transcript로 정리했습니다.

---

## 부록: 주요 파일 경로

| 경로 | 설명 |
|------|------|
| `docs/test_plan.md` | step-3 공식 산출물 (docs) |
| `Report/step-3.test_plan.md` | step-3 Report 보고서 |
| `Report/step-1.requirements_analysis.md` | step-1 요구사항 분석 (T-01~T-28 기준) |
| `Report/step-2.code_quality.md` | step-2 코드 품질 (참조) |
| `tasks/step-3_test-plan.md` | 과제 지시문 |
| `Prompting/step-3-test-plan-prompt.md` | 본 Export Transcript |

## 부록: TDD Phase · 우선순위

| Phase | 타겟 | 테스트 ID | 내용 |
|-------|------|-----------|------|
| A | `fake_tuner_test` | T-01 ~ T-06 | FakeTuner 경계·seek wrap |
| B | `controller_test` | T-07 ~ T-14 | 기능 1: 숫자·확정·Other·예외 |
| C | `controller_test` | T-15 ~ T-18 | 기능 2: 선호 토글·정렬 |
| D | `controller_test` | T-19 ~ T-23 | 기능 3: next favorite·wrap |
| E | `controller_mock_test` | T-24 ~ T-28 | setCH 횟수·순서·미호출 |
| F | (리팩토링) | lcov | ≥ 80% (권장 90%) |

| 등급 | 테스트 ID |
|------|-----------|
| P0 | T-01~04, T-07~08, T-14 |
| P1 | T-05~06, T-09~13, T-15~21, T-24~26, T-28 |
| P2 | T-17, T-22~23, T-27 |

## 부록: 테스트 시나리오 번호 (T-01 ~ T-28)

| 구간 | 파일 | 건수 |
|------|------|------|
| T-01 ~ T-06 | `FakeTunerTest.cpp` (`TEST`) | 6 |
| T-07 ~ T-23 | `TVChannelControllerTest.cpp` (`TEST_F`) | 17 |
| T-24 ~ T-28 | `TVChannelControllerMockTest.cpp` (`TEST_F`) | 5 |

## 부록: Fake vs Mock 분리 (요약)

| 구분 | Fake (`controller_test`) | Mock (`controller_mock_test`) |
|------|----------------------------|-------------------------------|
| 검증 | 최종 채널·`favorites_`·예외 | `setCH`/`getCurrentCH` 호출·횟수·순서 |
| 대표 패턴 | `EXPECT_EQ(tuner.getCurrentCH(), "12")` | `EXPECT_CALL(mock, setCH("12")).Times(1)` |
| 순서 | 불필요 | `InSequence` (T-27) |
| 미호출 | 채널 문자열 동일 | `EXPECT_CALL(setCH(_)).Times(0)` (T-28) |
| 중복 금지 | Mock에 favorites 검증 X | Fake에 `Times(1)` 남용 X |

## 부록: 경계값 ID (요약)

| ID | 값 / 상태 | 시나리오 |
|----|-----------|----------|
| B-CH-00 ~ 99 | 0, 1, 98, 99 | T-01, T-02, T-07, T-12, T-13 |
| B-CH--1, 100 | -1, 100 | T-03, T-04, T-14 |
| B-FAV-0, 1 | 빈·단일 favorites | T-21, T-23, T-28 |
| inputBuffer_ | -1 Idle, 0~9 Buffered | T-07, T-10, F1-10 |
| seek / next wrap | `available_` 끝, fav 끝 | T-06, T-20 |

## 부록: 커버리지·lcov

| 지표 | 목표 |
|------|------|
| README | 라인 **≥ 80%** |
| 권장 | **≥ 90%** (`TVChannelController.cpp`) |
| 측정 | gcov + lcov (`ENABLE_COVERAGE` CMake 옵션) |
| 미커버 보완 | Confirm+Idle, Favorite erase, wrap, T-14, 채널 98 |

## 부록: 요구사항 ↔ 테스트 교차 참조

| 요구사항 | 테스트 ID |
|----------|-----------|
| F1-01 ~ F1-10 | T-07 ~ T-14 |
| F2-01 ~ F2-04 | T-15 ~ T-18 |
| F3-01 ~ F3-06 | T-19 ~ T-23, T-28 |
| FT-01 ~ FT-06 | T-01 ~ T-06 |
| MK-01 ~ MK-06 | T-24 ~ T-28 |
