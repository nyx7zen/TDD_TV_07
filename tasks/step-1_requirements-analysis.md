@README.md @include/ITuner.h @include/TVChannelController.h @include/FakeTuner.h @include/MockTuner.h

[P] 시니어 C++ QA 엔지니어 관점에서
[C] TDD TV Channel Controller 프로젝트 (C++17, CMake, Google Test / GMock, Test Double: Fake/Mock)
[T] README.md 과제 명세를 C++ 구현·테스트 관점에서 재정리해줘.
    1) 기능 영역별 비즈니스 규칙 표
       - 숫자 입력·확정 (기능 1)
       - 선호 채널 토글 (기능 2)
       - 다음 선호 채널 (기능 3)
       - FakeTuner (ITuner 테스트 더블) 동작
       - MockTuner 검증 관점 (행위 기반)
    2) 채널 문자열·정수 변환·분기 시 주의점
       (std::stoi / std::to_string, "0"~"99", 선행 0, 버퍼·즉시 적용 등)
    3) 예외·경계값·상태 조건
       (채널 0~99, 100/-1, inputBuffer_ -1, 빈 favorites_, wrap-around)
    4) Test Double 사용 규칙 명세
       (FakeTuner: 상태 검증 / MockTuner: EXPECT_CALL·호출 순서·미호출)
    5) Google Test 기준 테스트 시나리오 목록 (번호)
       README TO-DO의 FakeTunerTest / TVChannelControllerTest / TVChannelControllerMockTest 전 항목 포함
[F] Markdown (표 + 번호 목록). `docs/requirements_analysis.md`에도 저장
