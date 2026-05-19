# Step 7 — Refactoring Plan (Export Transcript)
_Exported on Tuesday, May 19, 2026 from Cursor_

---

## Turn 1

**User**

@step-7_refactoring-plan.md (1-20)

**과제 요약 (tasks/step-7_refactoring-plan.md)**

- [P] 모던 C++ 리팩토링 코치
- [C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
- [T] TVChannelController 리팩토링 계획을 단계별로 제안
  - **제약:** `ITuner.h` 수정 금지, 채널 0~99, 모든 단계 테스트 Green (`dev/refactoring`, README refactor TODO)
  - **내용:**
    - 조건 분기 축소·중복 제거 (커밋 단위: `clearBuffer`, `findNextFavorite`, `addToFavorites`, `applyChannel` 등)
    - 타입·책임 분리 (`ChannelInputBuffer`, `FavoriteChannelList`, Command·Handler, `std::upper_bound`)
    - 매직 넘버 상수화·C++17 (`-1`, 0, 99, 10, digit; FakeTuner 검증 중복 정리)
    - Fake vs Mock 회귀 영향 점검
- [F] 단계별 체크리스트 + 검증 (`cmake --build build && ctest --test-dir build`), `docs/refactoring_plan.md` 저장

**참조 파일:** `README.md`, `tasks/step-1_requirements-analysis.md`, `tasks/step-2_code-quality.md`, `Report/code_quality_report.md`, `Report/requirements_analysis.md`, `include/TVChannelController.h`, `src/TVChannelController.cpp`, `include/FakeTuner.h`

---

**Cursor**

Step 7 리팩토링 계획 수립을 위해 코드베이스·품질 보고서·README refactor TODO·테스트 현황을 확인했다.

**수행한 작업**

1. **현재 구현 분석**
   - `TVChannelController.h` / `.cpp`: Green 구현 확인 (`applyChannel` 단일 관문, `pressNumber` 상태 머신, `upper_bound` 선호 채널)
   - `FakeTuner.h`: `setCH` 0~99 방어적 검증 (컨트롤러 `isValidChannel`과 DRY 이슈)
   - `docs/code_quality_report.md`, `Report/step-2.code_quality.md`: SRP·중복·우선순위 2~4 반영

2. **기준선 검증**
   ```powershell
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```
   - **4/4 Passed:** `fake_tuner_test`, `controller_test`, `controller_mock_test`, `golden_test`

3. **핫스팟·회귀 주의 식별**
   - `inputBuffer_ = -1` 다중 대입 → `clearBuffer()` (Phase 2)
   - `pressNextFavorite` 인라인 → `findNextFavorite()` (Phase 4)
   - `pressFavorite` / `addFavorite` 이중 경로 → `addToFavorites()` (Phase 5)
   - **`next == 10` 보류 분기:** 채널 10 즉시 적용 안 함 — Golden `channel_*` 회귀 주의

4. **단계별 계획 문서 작성**
   - `docs/refactoring_plan.md` 생성
   - Phase 0~10 (커밋 단위), README TODO 매핑, Fake/Mock/Golden 회귀 매트릭스, 표준 검증 명령, DoD, 목표 Facade 구조

**응답 요약 (Turn 1)**

| 구분 | 결과 |
|------|------|
| 산출 | `docs/refactoring_plan.md` |
| ctest 기준선 | **4/4 Pass** |
| 코드 변경 | **없음** (계획만) |
| Phase 수 | 0~10 (+ 선택 Phase 8 Command) |
| `ITuner.h` | **미수정** |

**권장 커밋 순서 (요약)**

```
Phase 0  기준선 → Phase 1 ChannelLimits → Phase 2 clearBuffer
→ Phase 3 applyChannel → Phase 4 findNextFavorite → Phase 5 addToFavorites
→ Phase 6 pressNumber → Phase 7A/7B 클래스 추출 → Phase 9~10 테스트·lcov
```

---

## Turn 2

**User**

Report 폴더에 보고서를 작성해서 저장해 주세요. 파일명은 step- 으로 시작

현재까지의 대화내용 전체를 Prompting 폴더에 Export Transcript 해 주세요. 파일명은 step- 으로 시작

작업한 모든 파일을 깃헙에 업로드 해 주세요.

---

**Cursor**

**수행 예정 작업**

1. `Report/step-7.refactoring_plan.md` — Step 7 Report (요약·요구 대응·Phase·회귀 전략)
2. `Prompting/step-7-refactoring-plan-prompt.md` — 본 Export Transcript (Turn 1~2 전체)
3. Git: `docs/refactoring_plan.md`, Report, Prompting 추가 → commit → `git push origin prompting`

---

## 부록: Step 7 산출 파일

| 파일 | 설명 |
|------|------|
| `docs/refactoring_plan.md` | 실행용 상세 Phase 체크리스트 |
| `Report/step-7.refactoring_plan.md` | Step 7 Report |
| `Prompting/step-7-refactoring-plan-prompt.md` | 본 Export Transcript |
| `tasks/step-7_refactoring-plan.md` | 과제 지시문 |

## 부록: 현재 코드 스냅샷 (분석 시점)

**TVChannelController 멤버 (요지)**

- `ITuner& tuner_`
- `int inputBuffer_ = -1`
- `std::vector<int> favorites_`
- private: `isValidChannel`, `isFavorite`, `applyChannel`
- public: `pressNumber`, `pressConfirm`, `pressFavorite`, `pressNextFavorite`, `pressOther`, `addFavorite`, `getFavoriteChannels`

**applyChannel (단일 관문)**

```cpp
void TVChannelController::applyChannel(int ch) {
    if (!isValidChannel(ch))
        throw std::invalid_argument("채널 범위 초과: " + std::to_string(ch));
    tuner_.setCH(std::to_string(ch));
}
```

## 부록: 검증 명령

```powershell
cd c:\DEV\week2_day2\TDD_TV_07
cmake --build build
ctest --test-dir build --output-on-failure
```

| executable | 역할 |
|--------------|------|
| `fake_tuner_test` | FakeTuner 경계 |
| `controller_test` | Fake 상태 기반 T-07~T-23 |
| `controller_mock_test` | Mock 협력 T-24~T-28 |
| `golden_test` | 12 Golden 시나리오 |

## 부록: Fake vs Mock vs Golden

| 종류 | 검증 대상 | 리팩토링 민감도 |
|------|-----------|-----------------|
| Fake | `getCurrentCH()`, 즐겨찾기 목록 | 숫자 입력·토글 |
| Mock | `setCH`/`getCurrentCH` 호출 | `applyChannel` 문자열·횟수 |
| Golden | 전체 trace 문자열 | `pressNumber` 상태 머신 |

## 부록: Prompting · Report 시리즈

| 파일 | 단계 |
|------|------|
| `step-1-requirements-analysis-prompt.md` | Step 1 |
| `step-2-code-quality-prompt.md` | Step 2 |
| `step-3-test-plan-prompt.md` | Step 3 |
| `step-4-write-unit-tests-prompt.md` | Step 4 |
| `step-5A-debug-test-failures-prompt.md` | Step 5A |
| `step-5B-defect-list-prompt.md` | Step 5B |
| `step-6-golden-master-prompt.md` | Step 6 |
| `step-7-refactoring-plan-prompt.md` | Step 7 (본 파일) |
| `Report/step-7.refactoring_plan.md` | Step 7 Report |

## 부록: 후속 단계

| Step | 내용 |
|------|------|
| Step 7 구현 | `dev/refactoring` — Phase 1부터 실제 리팩토링 |
| Step 8~9 | QA·결함 보고 |

`ITuner.h` — **미수정** (Step 7 계획·문서 단계).
