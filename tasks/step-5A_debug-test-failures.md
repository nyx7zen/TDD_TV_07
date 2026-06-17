@README.md @tasks/step-1_requirements-analysis.md @include/ITuner.h @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h @test/FakeTunerTest.cpp @test/TVChannelControllerTest.cpp @test/TVChannelControllerMockTest.cpp

[P] 디버깅과 결함 분석에 능한 C++ QA 엔지니어입니다.
[C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
[T] (여기에 `ctest --test-dir build` 또는 `ctest --output-on-failure` 실패 로그를 붙여넣을 것)
    1) EXPECT_EQ / ASSERT_* / EXPECT_CALL 실패의 기대·실제 차이 요약
       (채널 문자열, favorites_ 목록, Mock 호출 횟수·인자·순서)
    2) 결함 위치 특정 (파일명:줄번호)
       (TVChannelController.cpp, FakeTuner.h, 해당 테스트 파일)
    3) 결함 심각도 (Critical/Major/Minor/Info) 분류 및 근거
    4) 최소 변경 수정 방안 (C++17 스타일)
       - 단, `ITuner.h` 수정 금지 (외부 제공 인터페이스)
       - Test Double·테스트 시나리오 의도는 유지
[F] 수정 diff 제안 + `cmake --build build && ctest --test-dir build` Green 확인 절차
