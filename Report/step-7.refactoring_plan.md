# TDD TV Channel Controller — 리팩토링 계획 (Step 7)

| 항목 | 내용 |
|------|------|
| 단계 | Step 7 — TVChannelController 리팩토링 계획 |
| 역할 | 모던 C++ 리팩토링 코치 |
| 프로젝트 | TDD TV Channel Controller (C++17, CMake, Google Test / GMock) |
| 기준 문서 | `README.md` dev/refactoring, `docs/code_quality_report.md`, `docs/requirements_analysis.md`, `Report/step-2.code_quality.md`, `Report/step-6.golden_master.md` |
| 산출물 | `docs/refactoring_plan.md`, `Report/step-7.refactoring_plan.md`, `Prompting/step-7-refactoring-plan-prompt.md` |
| 검증 명령 | `cmake --build build && ctest --test-dir build --output-on-failure` |
| 작성일 | 2026-05-19 |
| 선행 단계 | Step 4 Green (28 단위 테스트), Step 6 Golden Master (12 시나리오, ctest 4/4) |
| 후속 단계 | `dev/refactoring` 브랜치에서 Phase 1~10 **실제 코드 리팩토링** 수행 |
| 제약 | `ITuner.h` **수정 없음**, 채널 **0~99** 유지, 매 Phase마다 테스트 Green |

---

## 1. 요약

Step 7은 **프로덕션 코드 변경 없이** `TVChannelController`·`FakeTuner`에 대한 **단계별 리팩토링 로드맵**을 수립했다. README `dev/refactoring` TODO, Step 2 코드 품질 분석, Step 6 Golden Master 회귀 기준선을 통합하여 **커밋 단위 Phase 0~10**과 Fake/Mock/Golden 회귀 매트릭스를 정의했다.

| 구분 | Step 6 (Golden) | Step 7 (본 단계) |
|------|-----------------|------------------|
| 목적 | E2E 스냅샷 회귀 방어 | 구조 개선 **계획**·체크리스트 |
| 코드 변경 | Golden 테스트·인프라 추가 | **계획 문서만** (구현은 후속) |
| 산출 | `test/golden/*`, `golden_test` | `docs/refactoring_plan.md` |
| ctest | 4/4 Pass (기준선) | 동일 **4/4 Pass** 확인 |

**검증 결과 (로컬, 2026-05-19):**

```text
ctest --test-dir build --output-on-failure

100% tests passed, 0 tests failed out of 4
  fake_tuner_test, controller_test, controller_mock_test, golden_test
```

---

## 2. 과제 요구사항 대응

| # | 요구 (Step 7) | 산출 | 근거 |
|---|---------------|------|------|
| 1 | 조건 분기 축소·중복 제거 (커밋 단위) | Phase 2~6: `clearBuffer`, `findNextFavorite`, `addToFavorites`, `pressNumber` 헬퍼 | `docs/refactoring_plan.md` §Phase 2~6 |
| 2 | 타입·책임 분리 | Phase 7: `ChannelInputBuffer`, `FavoriteChannelList`; Phase 8(선택): Command | §Phase 7~8 |
| 3 | 매직 넘버 상수화·C++17 | Phase 1: `ChannelLimits` | §Phase 1 |
| 4 | Fake vs Mock 회귀 영향 | 회귀 매트릭스 (Fake 상태 / Mock 협력 / Golden E2E) | §Fake vs Mock |
| 5 | 단계별 체크리스트·검증 | Phase 0~10 각각 `ctest` 명령 | §표준 검증 |
| 6 | `docs/refactoring_plan.md` 저장 | **완료** | 저장 경로 |
| — | `ITuner.h` 수정 금지 | 계획 전 구간 **미수정** 유지 명시 | 제약 표 |

---

## 3. 현재 코드 진단 (리팩토링 출발점)

### 3.1 구현 상태

`TVChannelController.cpp`는 Step 4 TDD Green 이후 **전 메서드 구현 완료** 상태이다 (`pressNumber` 상태 머신, `applyChannel` 단일 `setCH` 관문, `pressNextFavorite`의 `upper_bound` + wrap).

### 3.2 핫스팟

| 위치 | 이슈 | README refactor |
|------|------|-----------------|
| `inputBuffer_ = -1` 다수 | Duplicated Code | `clearBuffer()` |
| `pressNextFavorite` 인라인 | Long Method | `findNextFavorite()` |
| `pressFavorite` vs `addFavorite` | 이중 경로 | `addToFavorites()` |
| `0`/`99`/`-1`/`10` 리터럴 | Magic Number, DRY | `ChannelLimits` |
| `pressNumber` 분기 | 조건문 복잡도 | `ChannelInputBuffer` (후기) |

### 3.3 회귀 주의: `next == 10` 보류

`pressNumber`에서 두 자리 조합 `next == 10`일 때 **즉시 `applyChannel`하지 않고** `inputBuffer_ = 10`으로 보류한다. 채널 10은 `1`→`0` 후 `pressConfirm()` 또는 추가 숫자로 확정된다. Phase 6·7에서 **반드시** T-07~T-14 및 Golden `channel_*` 시나리오로 재검증한다.

```cpp
// src/TVChannelController.cpp (요지)
} else if (next == 10) {
    inputBuffer_ = 10;
} else {
    applyChannel(next);
    inputBuffer_ = -1;
}
```

---

## 4. 단계별 계획 (Phase 0~10)

각 Phase는 **한 커밋 = 한 논리 변경**. 실패 시 `git revert` 후 표준 검증.

| Phase | 작업 | 커밋 예시 |
|-------|------|-----------|
| **0** | `dev/refactoring` 기준선, ctest 4/4·Golden 고정 | (브랜치만) |
| **1** | `ChannelLimits` 공유 상수 | `refactor: extract ChannelLimits` |
| **2** | `clearBuffer()` | `refactor: extract clearBuffer` |
| **3** | `applyChannel` 파이프라인·`isValidChannel` 캡슐화 | `refactor: consolidate applyChannel` |
| **4** | `findNextFavorite()` | `refactor: extract findNextFavorite` |
| **5** | `addToFavorites` / `removeFromFavorites` | `refactor: favorite list CRUD` |
| **6** | `pressNumber` 헬퍼·분기 축소 | `refactor: simplify pressNumber` |
| **7A** | `ChannelInputBuffer` 클래스 | `refactor: extract ChannelInputBuffer` |
| **7B** | `FavoriteChannelList` 클래스 | `refactor: extract FavoriteChannelList` |
| **8** | (선택) Command/Handler | — |
| **9** | 테스트 픽스처·GWT·이름 | `refactor: test readability` |
| **10** | lcov 80%+, README 체크리스트 | — |

상세 체크리스트·검증 시나리오는 `docs/refactoring_plan.md`를 따른다.

---

## 5. README dev/refactoring TODO 매핑

| README 항목 | Phase |
|-------------|-------|
| `clearBuffer()` | 2 |
| `findNextFavorite()` | 4 |
| `isValidChannel` → `applyChannel` 전용 | 3 |
| `isFavorite` 재사용 | 5 |
| `applyChannel` 파이프라인 | 3 |
| `addToFavorites()` | 5 |
| `addFavorite` / 내부 중복 제거 | 5 |
| `ChannelInputBuffer` / `FavoriteChannelList` | 7 |
| 테스트 픽스처·GWT·이름 | 9 |
| lcov 80%+ | 10 |

---

## 6. Fake vs Mock vs Golden 회귀 전략

| 테스트 | 역할 | 리팩토링 시 확인 |
|--------|------|------------------|
| **Fake** (`controller_test`) | 최종 `getCurrentCH()`, `getFavoriteChannels()` | T-07~T-23 상태 |
| **Mock** (`controller_mock_test`) | `setCH`/`getCurrentCH` **횟수·순서·인자** | `applyChannel` 경유 문자열 불변 |
| **Golden** (`golden_test`) | 전체 `CH=… FAV=[…]` 트레이스 | `pressNumber` 변경 시 **최우선** |

**불변식:** `applyChannel`만 `ITuner::setCH`를 호출 → Mock 계약 단순·안정.

| 변경 유형 | Fake | Mock | Golden |
|-----------|------|------|--------|
| `ChannelLimits` | 경계 테스트 | 낮음 | 없음 |
| `clearBuffer` | T-10 | 없음 | `channel_buffer_cleared_by_other` |
| `ChannelInputBuffer` | 숫자 입력 전체 | `setCH("12")` 등 | `channel_*` 대부분 |

---

## 7. 목표 아키텍처 (Phase 7 완료 후)

```
TVChannelController (Facade)
├── ITuner& tuner_
├── ChannelInputBuffer buffer_
├── FavoriteChannelList favorites_
└── applyChannel(int)   // sole setCH + validation gateway
```

---

## 8. 표준 검증·완료 정의

### 8.1 표준 검증 (모든 Phase)

```powershell
cd c:\DEV\week2_day2\TDD_TV_07
cmake --build build
ctest --test-dir build --output-on-failure
```

### 8.2 Definition of Done (리팩토링 **실행** 완료 시)

- [ ] `ITuner.h` 미수정
- [ ] 채널 0~99, 예외·Golden·T-14 동작 동일
- [ ] `ctest` 4/4 Pass
- [ ] README dev/refactoring 체크리스트 전항목 [x]
- [ ] lcov `TVChannelController` **80%+**
- [ ] 커밋 이력 ↔ `docs/refactoring_plan.md` Phase 대응

Step 7 본문 시점에서는 **계획 수립만 완료**이며, 위 DoD는 `dev/refactoring` 구현 단계의 목표이다.

---

## 9. 리스크·롤백

| 리스크 | 완화 |
|--------|------|
| `next==10` 로직 훼손 | Golden `channel_*` + T-07~T-14 |
| Mock `setCH` 문자열 변경 | `std::to_string` 포맷 고정 |
| `addFavorite` public API 제거 | 구현만 `addToFavorites` 위임 |
| 클래스 추출 순환 의존 | `ChannelInputBuffer`는 `ITuner` 비의존 |

**롤백:** Phase별 커밋 → `git revert` → 표준 검증.

---

## 10. 산출물·문서 맵

| 파일 | 용도 |
|------|------|
| `docs/refactoring_plan.md` | 실행용 상세 체크리스트 (Phase 0~10) |
| `Report/step-7.refactoring_plan.md` | 본 Report (요약·대응표) |
| `Prompting/step-7-refactoring-plan-prompt.md` | Cursor 대화 Export Transcript |
| `tasks/step-7_refactoring-plan.md` | 과제 지시문 |

---

## 11. 후속 단계

| Step | 내용 |
|------|------|
| Step 7 구현 | `dev/refactoring`에서 Phase 1부터 순차 적용 |
| Step 8~9 | 결함 보고·QA 최종 (`tasks/step-9_*`) |

Golden Master(Step 6)와 단위 테스트(Step 4)를 **회귀 기준선**으로 유지한 채 README refactor TODO를 Phase 순으로 소진한다.
