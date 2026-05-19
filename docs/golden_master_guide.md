# Golden Master (Approval) 회귀 테스트 가이드

README 시나리오를 **텍스트 스냅샷**으로 고정해, `TVChannelController` 동작이 바뀌면 Golden Master 테스트가 실패하도록 구성했다.

## 구성 요약

| 항목 | 위치 |
|------|------|
| 시나리오 스크립트 | `test/golden/<name>.scenario` |
| 기대 출력 (Golden) | `test/golden/<name>.approved.txt` |
| 실행기 / 비교 로직 | `test/golden_master/ScenarioRunner.h` |
| Google Test | `test/TVChannelGoldenTest.cpp` |
| CMake 타깃 | `golden_test` |
| ctest 등록 | `golden_test` (Windows: executable 단위, Linux: `gtest_discover_tests`) |

`ITuner.h`는 수정하지 않는다. `FakeTuner`만 사용한다.

---

## 1. 스냅샷 출력 형식

각 **단계 직후** (초기 상태 1줄 + 액션마다 1줄) 다음 한 줄을 덤프한다.

```text
CH=<현재채널> FAV=[<오름차순 즐겨찾기, 쉼표 구분>]
```

예:

```text
CH=12 FAV=[6,12,37]
CH=0 FAV=[]
```

예외가 발생하는 시나리오는 마지막 줄에 다음을 추가한다.

```text
!EXCEPTION <메시지>
```

---

## 2. 시나리오 스크립트 DSL (`.scenario`)

한 줄에 하나의 액션. `#` 로 시작하는 줄과 빈 줄은 무시한다.

| 토큰 | 의미 | 대응 API |
|------|------|----------|
| `N0` … `N9` | 숫자 버튼 | `pressNumber(digit)` |
| `N <d>` | 숫자 버튼 (공백 형식) | `pressNumber(d)` |
| `C` / `CONFIRM` | 확정 | `pressConfirm()` |
| `F` / `FAVORITE` | 즐겨찾기 토글 | `pressFavorite()` |
| `NF` / `NEXT_FAVORITE` | 다음 즐겨찾기 | `pressNextFavorite()` |
| `O` / `OTHER` | 기타 (버퍼 취소) | `pressOther()` |
| `T <ch>` | 채널 이동 (테스트 헬퍼) | `pressNumber` + `pressConfirm` 또는 두 자리 입력 |
| `@CH <ch>` | 튜너 직접 설정 | `tuner.setCH(...)` |
| `@FAV 1,4,12` | 즐겨찾기 사전 등록 | `addFavorite` 반복 |

### 샘플: `channel_single_confirm.scenario`

```text
# T-01: single digit + confirm
N1
C
```

### 샘플: `channel_single_confirm.approved.txt`

```text
CH=0 FAV=[]
CH=0 FAV=[]
CH=1 FAV=[]
```

---

## 3. Golden 파일 보관 전략

- **디렉터리:** `test/golden/` (소스 트리에 커밋)
- **이름 규칙:** `<시나리오명>.scenario` + `<시나리오명>.approved.txt`
- **비교 방식:** 시나리오 실행 후 전체 출력 문자열을 `.approved.txt`와 **완전 일치** 비교 (`EXPECT_EQ`)
- **경로:** CMake가 `GOLDEN_DIR`을 `golden_test`에 compile definition으로 주입 (`CMAKE_SOURCE_DIR/test/golden`)

새 README 시나리오를 추가할 때:

1. `test/golden/<new_name>.scenario` 작성
2. `TVChannelGoldenTest.cpp`에 `TEST_F(..., NewName)` 추가
3. Golden 생성 (아래 “갱신 절차”)
4. `*.approved.txt`를 커밋

---

## 4. 빌드 및 실행

### 로컬 (Windows / MinGW 예시)

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`golden_test`만 실행:

```powershell
.\build\golden_test.exe
```

### Linux / macOS

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
# 또는 개별 케이스: ./build/golden_test --gtest_filter=GoldenMasterTest.*
```

---

## 5. Golden 갱신 절차 (의도적 동작 변경 시)

동작을 **의도적으로** 바꾼 뒤 기대 스냅샷을 다시 받을 때만 사용한다.

### Windows (PowerShell)

```powershell
$env:UPDATE_GOLDENS = "1"
.\build\golden_test.exe
Remove-Item Env:UPDATE_GOLDENS
```

### Linux / macOS

```bash
UPDATE_GOLDENS=1 ./build/golden_test
```

- `UPDATE_GOLDENS=1` 이면 해당 테스트는 `.approved.txt`를 덮어쓰고 **Skipped** 처리된다.
- 이후 `UPDATE_GOLDENS` 없이 다시 실행해 **12/12 Passed** 인지 확인한다.
- 변경된 `.approved.txt` diff를 리뷰한 뒤 커밋한다.

---

## 6. 포함된 시나리오 (README 매핑)

| Golden 이름 | README / T-ID |
|-------------|----------------|
| `channel_single_confirm` | T-01 한 자리 + 확정 |
| `channel_two_digit_immediate` | T-02 두 자리 즉시 적용 |
| `channel_sequence_12_then_34` | T-03 연속 두 자리 |
| `channel_buffer_cleared_by_other` | T-04 OTHER 버퍼 취소 |
| `channel_leading_zero_07` | T-05 선행 0 |
| `channel_99_boundary` | T-07 채널 99 |
| `favorites_complex_toggle` | T-12 복합 즐겨찾기 |
| `next_favorite_from_6` | T-16 다음 즐겨찾기 |
| `next_favorite_wrap_56_to_1` | T-17 wrap-around |
| `next_favorite_empty_list` | T-18 빈 목록 |
| `exception_channel_100` | T-08 채널 100+ 예외 |
| `readme_smoke_end_to_end` | 기능 1~3 통합 스모크 |

---

## 7. CI

GitHub Actions: [`.github/workflows/ci.yml`](../.github/workflows/ci.yml)

- `ubuntu-latest`에서 CMake + Ninja로 빌드
- `ctest --output-on-failure`로 `fake_tuner_test`, `controller_test`, `controller_mock_test`, **`golden_test`** 실행
- PR/ push to `main`·`dev` 시 자동 실행

로컬에서 CI와 동일하게 검증하려면:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 8. 실패 시 디버깅

1. `golden_test` 단독 실행으로 어떤 시나리오가 깨졌는지 확인
2. 해당 `.scenario`와 `.approved.txt` diff 확인
3. 의도된 변경이면 `UPDATE_GOLDENS=1`로 스냅샷 갱신 후 diff 리뷰
4. 버그면 프로덕션 코드 수정 — Golden은 **회귀 방어막**이므로 무분별 갱신 금지
