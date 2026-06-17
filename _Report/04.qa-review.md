# TDD TV Channel Controller — 코드베이스 QA 종합 리뷰

| 항목 | 내용 |
|---|---|
| 리뷰 기준 | `README.md` TO-DO, AAA 패턴, 경계값 테스트 |
| 리뷰 일자 | 2026-05-19 |
| 빌드·테스트 | CMake + GTest/GMock, CTest 3/3 통과 (스텁 수준) |

---

## 1. 요구사항 충족도 (README.md 기준 매핑)

`README.md`의 TO-DO를 **파일 준비**, **FakeTunerTest**, **TVChannelControllerTest (기능 1~3)**, **TVChannelControllerMockTest**, **리팩토링** 순으로 매핑했다.

### 1.1 파일 준비

| README 항목 | 상태 | 근거 |
|---|---|---|
| `ITuner.h` 확인 (수정 불가) | ✅ 충족 | 순수 가상 인터페이스 3메서드 정의 |
| `FakeTuner.h` 작성 | ✅ 충족 | `setCH` 범위 검증, `seekCH` wrap-around 구현 |
| `MockTuner.h` 작성 | ✅ 충족 | `MOCK_METHOD` 3개 선언 |
| `TVChannelController.h` 작성 | ✅ 충족 | `ITuner&`, `inputBuffer_`, `favorites_`, 헬퍼 선언 |
| `TVChannelController.cpp` 스텁 | ⚠️ 부분 | 컴파일은 되나 **모든 public 메서드가 빈 구현** |
| `CMakeLists.txt` 테스트 3개 등록 | ✅ 충족 | `fake_tuner_test`, `controller_test`, `controller_mock_test` |

**파일 준비 충족률: 5/6 (약 83%)** — 구현체가 스텁이라 기능 요구는 미충족.

### 1.2 FakeTunerTest.cpp (6건)

| # | README 시나리오 | 상태 |
|---|---|---|
| 1 | `setCH("0")` → `getCurrentCH() == "0"` | ❌ 미구현 |
| 2 | `setCH("99")` → `getCurrentCH() == "99"` | ❌ 미구현 |
| 3 | `setCH("-1")` → `std::invalid_argument` | ❌ 미구현 |
| 4 | `setCH("100")` → `std::invalid_argument` | ❌ 미구현 |
| 5 | `seekCH()` 연속 호출 → 유효 채널 | ❌ 미구현 |
| 6 | `seekCH()` wrap-around | ❌ 미구현 |

현재 `FakeTunerTest.cpp`는 `Stub` 1건만 존재하며, 초기 채널 `"0"`만 검증한다. **FakeTunerTest 충족률: 0/6 (0%)**.

> 참고: `FakeTuner.h`에는 경계 검증·wrap-around **로직은 이미 구현**되어 있으나, README가 요구한 **테스트로 증명되지 않음**.

### 1.3 TVChannelControllerTest.cpp — 기능 1 (숫자 버튼, 8건)

| # | README 시나리오 | 상태 |
|---|---|---|
| 1 | `pressNumber(1)` + `pressConfirm()` → `"1"` | ❌ |
| 2 | `pressNumber(1,2)` → `"12"` | ❌ |
| 3 | `1,2,3,4` 연속 → 12 후 34 | ❌ |
| 4 | `4,5,6` + `pressOther()` → 변화 없음 | ❌ |
| 5 | `0,7` → `"7"` | ❌ |
| 6 | 채널 0 입력 | ❌ |
| 7 | 채널 99 입력 | ❌ |
| 8 | 채널 100+ → `std::invalid_argument` | ❌ |

**기능 1 충족률: 0/8 (0%)**.

### 1.4 TVChannelControllerTest.cpp — 기능 2 (선호 채널, 4건)

| # | README 시나리오 | 상태 |
|---|---|---|
| 1 | 미등록 `pressFavorite()` → 추가 | ❌ |
| 2 | 등록된 `pressFavorite()` → 삭제(토글) | ❌ |
| 3 | `12→★ 08→★ …` → `{6,12,37}` | ❌ |
| 4 | `getFavoriteChannels()` 정렬 검증 | ❌ |

**기능 2 충족률: 0/4 (0%)**.

### 1.5 TVChannelControllerTest.cpp — 기능 3 (다음 선호, 5건)

| # | README 시나리오 | 상태 |
|---|---|---|
| 1 | `{1,4,12,56}`, 현재 6 → `"12"` | ❌ |
| 2 | 현재 56 → `"1"` (wrap-around) | ❌ |
| 3 | 목록 비어 있을 때 변화 없음 | ❌ |
| 4 | 목록 외 값 → 다음 큰 선호 채널 | ❌ |
| 5 | 목록 1개 → wrap-around | ❌ |

**기능 3 충족률: 0/5 (0%)**.

### 1.6 TVChannelControllerMockTest.cpp (5건)

| # | README 시나리오 | 상태 |
|---|---|---|
| 1 | `pressNumber(1)` + `pressConfirm()` → `setCH("1")` 1회 | ❌ |
| 2 | `pressNumber(1,2)` → `setCH("12")` 1회 | ❌ |
| 3 | `pressFavorite()` → `getCurrentCH()` 호출 | ❌ |
| 4 | `pressNextFavorite()` → `getCurrentCH()` 후 `setCH()` 순서 | ❌ |
| 5 | 목록 비어 있을 때 `setCH()` 미호출 | ❌ |

**Mock 테스트 충족률: 0/5 (0%)**. 현재 `EXPECT_TRUE(true)` 수준의 스텁만 존재.

### 1.7 리팩토링 브랜치 (dev/refactoring)

`clearBuffer()`, `findNextFavorite()`, `addToFavorites()`, Given-When-Then 주석, lcov 80% 등 **전 항목 미착수** (구현·테스트 Green 상태 전제 미달).

### 1.8 종합 충족도

| 구분 | 완료 | 전체 | 비율 |
|---|---|---|---|
| 파일 준비 (구조·헤더) | 5 | 6 | 83% |
| FakeTunerTest | 0 | 6 | 0% |
| ControllerTest (기능 1~3) | 0 | 17 | 0% |
| MockTest | 0 | 5 | 0% |
| **테스트 시나리오 합계** | **0** | **28** | **0%** |
| **전체 TO-DO (테스트 중심)** | **5** | **34** | **약 15%** |

**판정:** 프로젝트 골격과 테스트 더블 설계는 README와 일치하나, **TDD의 Red→Green 사이클이 아직 시작되지 않은 초기 스캐폴딩 단계**이다. CTest 3/3 통과는 스텁이 사실상 검증하지 않기 때문에 QA 관점에서는 **통과 ≠ 요구사항 충족**으로 본다.

---

## 2. AAA 패턴 준수도

AAA(Arrange–Act–Assert)는 테스트를 **준비 → 실행 → 검증** 세 단계로 분리해 가독성과 의도를 명확히 하는 패턴이다.

### 2.1 현황

| 테스트 파일 | 테스트 수 | AAA 구조 | Given-When-Then 주석 |
|---|---|---|---|
| `FakeTunerTest.cpp` | 1 | ❌ 불완전 | ❌ 없음 |
| `TVChannelControllerTest.cpp` | 1 | ❌ 불완전 | ❌ 없음 |
| `TVChannelControllerMockTest.cpp` | 1 | ❌ 불완전 | ❌ 없음 |

### 2.2 파일별 분석

**FakeTunerTest.Stub**

```cpp
TEST(FakeTunerTest, Stub) {
    FakeTuner tuner({1, 4, 12, 56});   // Arrange (일부)
    EXPECT_EQ("0", tuner.getCurrentCH()); // Assert — Act 없음
}
```

- **Arrange:** `FakeTuner` 생성만 수행.
- **Act:** `setCH`, `seekCH` 등 동작 호출 없음.
- **Assert:** 초기값만 검증.

**TVChannelControllerTest.Stub**

```cpp
TEST(TVChannelControllerTest, Stub) {
    FakeTuner tuner({1, 4, 12, 56});
    TVChannelController ctrl(tuner);     // Arrange
    EXPECT_EQ("0", tuner.getCurrentCH()); // Assert — SUT(ctrl) 미사용, Act 없음
}
```

- SUT(`TVChannelController`)를 생성하지만 **어떤 메서드도 호출하지 않음**.
- 검증 대상이 **튜너 초기값**이어서 컨트롤러 행위와 무관.

**TVChannelControllerMockTest.Stub**

- `EXPECT_TRUE(true)` — Arrange/Act/Assert 모두 실질적으로 부재.

### 2.3 준수도 평가

| 평가 항목 | 결과 |
|---|---|
| 3단계 명시적 분리 | ❌ 0/3 테스트 |
| Act 단계(SUT 메서드 호출) | ❌ 없음 |
| Assert가 검증 목적과 일치 | ❌ Controller 테스트가 튜너만 검증 |
| README 권장 Given-When-Then 주석 | ❌ 전 파일 미적용 |
| `ASSERT_*` vs `EXPECT_*` 구분 | ❌ 적용 사례 없음 |

**AAA 준수도: 미충족 (0%)** — README 리팩토링 항목에서도 “Given-When-Then 주석 일관성”이 TODO로 남아 있으며, 현 코드베이스와 일치한다.

### 2.4 권장 AAA 템플릿 (참고)

```cpp
TEST(TVChannelControllerTest, ConfirmAppliesSingleDigitChannel) {
    // Arrange
    FakeTuner tuner({1, 4, 12, 56});
    TVChannelController ctrl(tuner);

    // Act
    ctrl.pressNumber(1);
    ctrl.pressConfirm();

    // Assert
    EXPECT_EQ("1", tuner.getCurrentCH());
}
```

---

## 3. 경계값 테스트 존재 여부

README와 도메인(채널 0~99)에서 요구하는 경계·예외 시나리오를 점검했다.

### 3.1 FakeTuner 경계 (README FakeTunerTest)

| 경계 유형 | 기대 시나리오 | 구현 (`FakeTuner.h`) | 테스트 |
|---|---|---|---|
| 최솟값 | `setCH("0")` | ✅ `0 <= v <= 99` | ❌ |
| 최댓값 | `setCH("99")` | ✅ | ❌ |
| 하한 미만 | `setCH("-1")` → 예외 | ✅ `invalid_argument` | ❌ |
| 상한 초과 | `setCH("100")` → 예외 | ✅ | ❌ |
| `seekCH` 순환 | 목록 끝 → 처음 | ✅ | ❌ |
| 빈 `available_` | (명시 없음) | ⚠️ `front()` UB 위험 | ❌ |

### 3.2 TVChannelController 경계 (README ControllerTest)

| 경계 유형 | 기대 시나리오 | 구현 | 테스트 |
|---|---|---|---|
| 채널 0 | 정상 변경 | ❌ 스텁 | ❌ |
| 채널 99 | 정상 변경 | ❌ 스텁 | ❌ |
| 채널 ≥100 | `invalid_argument` | ❌ (`applyChannel` 빈 함수) | ❌ |
| 선호 목록 비어 있음 | 채널 유지 | ❌ | ❌ |
| 선호 1개 wrap | 순환 | ❌ | ❌ |
| 다음 선호 wrap (56→1) | 순환 | ❌ | ❌ |
| 입력 버퍼 무효화 | `pressOther` 후 무변화 | ❌ | ❌ |
| 두 자리 입력 경계 | 12 → 34 순차 | ❌ | ❌ |

### 3.3 종합

| 구분 | README 경계 시나리오 수 | 테스트 존재 |
|---|---|---|
| FakeTuner | 6 | **0** |
| Controller | 11+ (기능별 경계 포함) | **0** |
| **합계** | **17+** | **0** |

**판정: 경계값 테스트 없음.**  
구현 측면에서 `FakeTuner::setCH`만 경계 로직이 있으나, **회귀 방지용 테스트가 없어** 리팩터 시 깨져도 CI가 잡지 못한다.

---

## 4. 개선 권고 사항

### 4.1 Critical (즉시 — 기능·TDD 핵심)

| ID | 항목 | 설명 | 권고 조치 |
|---|---|---|---|
| C-01 | `TVChannelController` 전 메서드 스텁 | README 기능 1~3 전부 미구현 | TDD: README 시나리오별 테스트 추가 후 `pressNumber`, `pressConfirm`, `applyChannel` 등 순차 구현 |
| C-02 | Controller 테스트가 SUT를 검증하지 않음 | `ctrl` 미호출, `tuner` 초기값만 assert | 모든 Controller 테스트에서 **Act = ctrl 메서드**, **Assert = tuner 상태 또는 favorites** |
| C-03 | README 테스트 28건 0% | 요구 시나리오 미반영 | `FakeTunerTest` 6건 → `TVChannelControllerTest` 17건 → `MockTest` 5건 순으로 Red 작성 |
| C-04 | Mock 테스트 미활용 | `EXPECT_TRUE(true)` | `EXPECT_CALL(mock, setCH("1")).Times(1)` 등 행위 검증 도입 |
| C-05 | `applyChannel` 미구현 | 채널 유효성·튜너 위임 없음 | 헤더의 `isValidChannel` + `tuner_.setCH` 연결, 100+ 예외 테스트와 함께 구현 |

### 4.2 Major (구조·품질)

| ID | 항목 | 설명 | 권고 조치 |
|---|---|---|---|
| M-01 | AAA·Given-When-Then 미적용 | 테스트 의도 불명확 | 각 테스트에 Arrange/Act/Assert 주석 또는 빈 줄로 구역 분리 |
| M-02 | 테스트 이름 `Stub` | 실패 시 원인 파악 어려움 | `동작_조건_기대결과` 패턴 (README 리팩토링 항목) |
| M-03 | `FakeTuner` 경계 로직 미검증 | 구현만 있고 테스트 없음 | README FakeTunerTest 6건 우선 Green |
| M-04 | `FakeTuner::seekCH` 빈 목록 | `available_.front()` UB | 빈 벡터 방어 또는 테스트로 전제 문서화 |
| M-05 | `available_` 정렬 미보장 | `seekCH` 동작이 입력 순서에 의존 | 생성자에서 `sort` 또는 README 전제 명시 + 테스트 |
| M-06 | 커버리지·lcov 미측정 | 80% 목표 미착수 | Green 후 lcov 도입, `applyChannel`·예외 분기 커버 |

### 4.3 Minor (유지보수·스타일)

| ID | 항목 | 설명 | 권고 조치 |
|---|---|---|---|
| N-01 | `ControllerTest` 픽스처 없음 | Arrange 중복 예상 | `ControllerTest : ::testing::Test` + `SetUp()` (README 리팩토링) |
| N-02 | `addFavorite` public | 테스트 헬퍼와 프로덕션 API 혼재 | 테스트 전용 빌드 플래그 또는 friend/픽스처 내부 헬퍼 검토 |
| N-03 | 예외 메시지 한글 | `"채널 범위 초과"` | 팀 컨벤션에 맞게 통일 (테스트는 `EXPECT_THROW` 타입만 검증 가능) |
| N-04 | `getFavoriteChannels()` const ref | 내부 컨테이너 노출 | 테스트 편의 vs 캡슐화 trade-off 문서화 |
| N-05 | 리팩토링 TODO 미진행 | `clearBuffer`, `findNextFavorite` 등 | 전 테스트 Green 후 README dev/refactoring 체크리스트 수행 |

---

## 5. 잘 구현된 부분

다음 항목은 README 의도와 TDD·SOLID 관점에서 **초기 단계로서 잘 갖춰진 부분**이다.

1. **의존성 역전 (`ITuner`)**  
   컨트롤러가 구체 튜너가 아닌 인터페이스에만 의존하며, `FakeTuner` / `MockTuner`로 상태·행위 검증을 분리할 수 있는 구조다.

2. **`FakeTuner` 도메인 로직**  
   `setCH`의 `0~99` 검증 및 `invalid_argument`, `seekCH`의 “다음 가용 채널 → 없으면 첫 채널” wrap-around는 README 명세와 부합한다.

3. **`MockTuner` GMock 선언**  
   `MOCK_METHOD` 3개가 `ITuner`와 일치해, 행위 검증 테스트를 바로 추가할 수 있다.

4. **`TVChannelController` 헤더 설계**  
   `inputBuffer_`, `favorites_`, `isValidChannel`, `isFavorite`, `applyChannel` 선언으로 책임 분리 방향이 README 리팩토링 목표와 맞다.

5. **CMake 테스트 분리**  
   Fake 단위 / Controller 상태 / Mock 행위의 **3 executable** 구성은 README 폴더 구조와 일치하며, 역할별 CI 확장에 유리하다.

6. **빌드·의존성 관리**  
   C++17, FetchContent로 GTest 1.16 고정, `gtest_discover_tests`로 CTest 연동이 정상 동작한다.

7. **테스트 더블 역할 분리 (설계 수준)**  
   `TVChannelControllerTest` → Fake(결과), `TVChannelControllerMockTest` → Mock(호출) 이원화는 README TDD 교육 목표에 부합한다.

---

## 6. QA 종합 판정

| 영역 | 등급 | 요약 |
|---|---|---|
| 요구사항 충족도 | **D** | 구조 83%, 시나리오 테스트 0% |
| AAA 패턴 | **F** | Act 부재, SUT 미검증 |
| 경계값 테스트 | **F** | 17+ 시나리오 중 0건 |
| 아키텍처·스캐폴딩 | **B+** | ITuner, Fake/Mock, CMake 양호 |
| 실행 가능성 | **A** | 빌드·CTest 통과 (의미 있는 검증은 아님) |

**다음 권장 작업 순서**

1. `FakeTunerTest.cpp` — README 6개 경계·wrap 테스트 (Red → Green).  
2. `TVChannelControllerTest.cpp` — 기능 1 숫자 입력부터 TDD.  
3. 기능 2·3 선호 채널 시나리오.  
4. `TVChannelControllerMockTest.cpp` — `EXPECT_CALL` 행위 검증.  
5. README `dev/refactoring` + lcov 80%.

---

*본 문서는 2026-05-19 기준 저장소 스냅샷(`TVChannelController` 스텁, 테스트 각 1건)을 기준으로 작성되었다.*
