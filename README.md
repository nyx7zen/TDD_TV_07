# TDD TV Channel Controller

- VSCode Java & C++ 개발자과정
- Test Double(Fake/Mock) 활용
- TDD 구현 C++ (Google Test / GMock) 위주

## 프로젝트 구조

```
TDD_TV_07/
├── CMakeLists.txt
├── README.md
├── coverage/
│   └── coverage_report.txt
├── include/
│   ├── ITuner.h
│   ├── FakeTuner.h
│   ├── MockTuner.h
│   └── TVChannelController.h
├── src/
│   └── TVChannelController.cpp
└── test/
    ├── FakeTunerTest.cpp
    ├── TVChannelControllerTest.cpp
    └── TVChannelControllerMockTest.cpp
```

## 테스트 현황 (dev 브랜치)

| 테스트 파일 | 테스트 수 | 상태 |
|---|---|---|
| FakeTunerTest | 6 | Passed |
| TVChannelControllerTest (기능1 숫자버튼) | 7 | Passed |
| TVChannelControllerTest (기능2 선호채널) | 4 | Passed |
| TVChannelControllerTest (기능3 다음선호채널) | 5 | Passed |
| TVChannelControllerMockTest | 5 | Passed |
| 합계 | 28 | 전체 Passed |

## 리팩토링 현황 (refactoring 브랜치)

| 항목 | 상태 |
|---|---|
| clearBuffer() 메서드 추출 | 완료 |
| addToFavorites() 메서드 추출 | 완료 |
| findNextFavorite() 메서드 추출 | 완료 |
| ASSERT_*/EXPECT_* 사용 기준 정리 | 완료 |
| Given-When-Then 주석 일관성 점검 | 완료 |
| 커버리지 확인 | 완료 |

## 커버리지 결과

| 파일 | 커버리지 |
|---|---|
| TVChannelController.cpp | 100% |
| TVChannelController.h | 100% |

## 브랜치 전략

- main: 초기 소스 (수정 금지)
- dev: TDD 테스트 작성 (완료)
- refactoring: 리팩토링 작업 (완료)

