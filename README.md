# TDD TV Channel Controller

- VSCode Java & C++ 개발자과정
— Test Double(Fake/Mock) 활용
- TDD 구현C++ (Google Test / GMock) 위주

첨부된 Gilded Rose 프로젝트 구조와 TDD TV 과제 명세를 함께 참고해서 실제로 생성해야 할 파일 기준으로 재검토합니다.

---

## 폴더 구조

```
TDD_TV_XX/
├── CMakeLists.txt
├── README.md
├── .vscode/
│   └── settings.json
├── include/
│   ├── ITuner.h                          # 외부 제공, 수정 불가 (순수 가상 인터페이스)
│   ├── FakeTuner.h                       # 상태 기반 테스트 대역, 직접 작성
│   ├── MockTuner.h                       # GMock MOCK_METHOD 기반, 직접 작성
│   └── TVChannelController.h             # 컨트롤러 헤더
├── src/
│   └── TVChannelController.cpp           # 컨트롤러 구현체
└── test/
    ├── FakeTunerTest.cpp                 # FakeTuner 자체 동작 검증
    ├── TVChannelControllerTest.cpp       # FakeTuner 사용, 상태 기반 검증
    └── TVChannelControllerMockTest.cpp   # MockTuner 사용, 행위 기반 검증
```

---

## TO-DO 리스트

### dev 브랜치 — 테스트 작성

#### 파일 준비

- [ ] `ITuner.h` 내용 확인 (수정 불가)
- [ ] `FakeTuner.h` 작성 (`current_`, `available_`, `seekCH` wrap-around, `setCH` 범위 예외)
- [ ] `MockTuner.h` 작성 (`MOCK_METHOD` 3개 선언)
- [ ] `TVChannelController.h` 작성 (멤버 선언: `ITuner&`, `inputBuffer_`, `favorites_`)
- [ ] `TVChannelController.cpp` 스텁 작성 (컴파일만 되는 빈 구현)
- [ ] `CMakeLists.txt` 에 테스트 실행 파일 3개 등록 확인

---

#### FakeTunerTest.cpp — FakeTuner 자체 동작 검증

- [ ] `test:` `setCH("0")` → `getCurrentCH() == "0"` (최솟값 경계)
- [ ] `test:` `setCH("99")` → `getCurrentCH() == "99"` (최댓값 경계)
- [ ] `test:` `setCH("-1")` → `std::invalid_argument` 예외
- [ ] `test:` `setCH("100")` → `std::invalid_argument` 예외
- [ ] `test:` `seekCH()` 연속 호출 → 항상 유효 채널 반환
- [ ] `test:` `seekCH()` wrap-around 동작 (목록 끝 → 목록 처음으로)

---

#### TVChannelControllerTest.cpp — 상태 기반 검증 (FakeTuner 사용)

기능 1 — 숫자 버튼 채널 변경

- [ ] `test:` `pressNumber(1)` + `pressConfirm()` → `getCurrentCH() == "1"`
- [ ] `test:` `pressNumber(1)` + `pressNumber(2)` → `getCurrentCH() == "12"`
- [ ] `test:` `1,2,3,4` 연속 입력 → 12번 후 34번 순차 변경
- [ ] `test:` `4,5,6` 입력 + `pressOther()` → 채널 변화 없음 (버퍼 무효화)
- [ ] `test:` `0,7` 입력 → `getCurrentCH() == "7"` (0으로 시작)
- [ ] `test:` 채널 0 입력 → 정상 변경
- [ ] `test:` 채널 99 입력 → 정상 변경
- [ ] `test:` 채널 100 이상 → `std::invalid_argument` 예외

기능 2 — 선호 채널 토글

- [ ] `test:` 미등록 채널 `pressFavorite()` → 목록에 추가
- [ ] `test:` 등록된 채널 `pressFavorite()` → 목록에서 삭제 (토글)
- [ ] `test:` `12→★ 08→★ 37→★ 08→★ 06→★` → 목록 `{6, 12, 37}`
- [ ] `test:` `getFavoriteChannels()` 반환 목록이 정렬 상태인지 검증

기능 3 — 다음 선호 채널

- [ ] `test:` 목록 `{1,4,12,56}`, 현재 6번 → `pressNextFavorite()` → `"12"`
- [ ] `test:` 목록 `{1,4,12,56}`, 현재 56번 → `pressNextFavorite()` → `"1"` (wrap-around)
- [ ] `test:` 목록 비어있을 때 → 채널 변화 없음
- [ ] `test:` 현재 채널이 목록 외 값일 때 → 현재보다 큰 첫 번째 선호 채널로 이동
- [ ] `test:` 목록에 채널 1개만 있을 때 → wrap-around 동작

---

#### TVChannelControllerMockTest.cpp — 행위 기반 검증 (MockTuner 사용)

- [ ] `test:` `pressNumber(1)` + `pressConfirm()` → `setCH("1")` 정확히 1회 호출
- [ ] `test:` `pressNumber(1)` + `pressNumber(2)` → `setCH("12")` 1회 호출
- [ ] `test:` `pressFavorite()` → `getCurrentCH()` 호출됨 검증
- [ ] `test:` `pressNextFavorite()` (목록 있을 때) → `getCurrentCH()` 후 `setCH()` 순서 검증
- [ ] `test:` `pressNextFavorite()` (목록 비어있을 때) → `setCH()` 호출 없음 검증

---

### dev/refactoring 브랜치 — 리팩토링

모든 테스트가 Green인 상태를 유지하면서 진행합니다.

#### TVChannelController 내부 구조 개선

- [ ] `refactor:` `inputBuffer_` 초기화 로직을 `clearBuffer()` 메서드로 추출 (`pressOther`, `pressConfirm`, 두 자리 완성 세 곳에서 공통 사용)
- [ ] `refactor:` `pressNextFavorite()` 의 `upper_bound` + wrap-around 로직을 `findNextFavorite()` 로 추출
- [ ] `refactor:` `isValidChannel()` 을 `applyChannel()` 내부 전용으로 책임 정리
- [ ] `refactor:` `isFavorite()` 헬퍼를 `pressFavorite()` / `pressNextFavorite()` 에서 재사용하도록 정리
- [ ] `refactor:` `applyChannel()` 에서 유효성 검사 → Tuner 호출 흐름을 단일 책임으로 정리

#### 선호 채널 목록 관리 개선

- [ ] `refactor:` `favorites_` push + sort 패턴을 `addToFavorites()` 로 추출
- [ ] `refactor:` `addFavorite()` 테스트 헬퍼와 내부 추가 로직 중복 제거

#### 테스트 코드 가독성 개선

- [ ] `refactor:` `ControllerTest` 픽스처 `SetUp()` 에서 중복 채널 설정 코드 통합
- [ ] `refactor:` Given-When-Then 주석 일관성 점검 및 누락 보완
- [ ] `refactor:` 테스트 이름을 `동작_조건_기대결과` 패턴으로 통일
- [ ] `refactor:` `ASSERT_*` (선제 조건) / `EXPECT_*` (결과 검증) 사용 기준 일관성 점검

#### 커버리지 확인

- [ ] lcov 측정 후 미커버 라인 확인 → 누락 테스트 보완
- [ ] 80% 이상 달성 확인 후 최종 push