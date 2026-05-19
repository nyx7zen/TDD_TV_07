@README.md @tasks/step-1_requirements-analysis.md @tasks/step-3_test-plan.md @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h @include/MockTuner.h @include/ITuner.h @test/FakeTunerTest.cpp @test/TVChannelControllerTest.cpp @test/TVChannelControllerMockTest.cpp @CMakeLists.txt

[P] 테스트 설계에 강한 시니어 C++ QA입니다.
[C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
[T] README TO-DO 시나리오를 충족하는 단위 테스트를 작성하고, 필요 시 프로덕션 코드를 구현해 Green을 만든다.
    - FakeTunerTest.cpp: README 6건 (경계 setCH 0/99/-1/100, seekCH·wrap-around)
    - TVChannelControllerTest.cpp: TEST_F 픽스처 사용, FakeTuner 기반 상태 검증
      · 기능 1 숫자 입력·확정 (8건)
      · 기능 2 선호 채널 토글 (4건)
      · 기능 3 다음 선호 채널 (5건)
    - TVChannelControllerMockTest.cpp: MockTuner + EXPECT_CALL 행위 검증 (5건)
    - 검증 방식
      · EXPECT_EQ / ASSERT_EQ: getCurrentCH(), getFavoriteChannels()
      · ASSERT_THROW(..., std::invalid_argument): 무효 채널
      · EXPECT_CALL / InSequence: setCH·getCurrentCH 호출 횟수·인자·순서·미호출
    - 경계값 포함 (채널 0/99/100/-1, digit 0·9, 빈·단일 favorites_, wrap-around)
    - Given-When-Then 주석 구조 사용
    - 테스트 이름: `동작_조건_기대결과` 패턴
[F] 완성된 테스트 코드 및 TVChannelController.cpp 구현.
    `cmake --build build && ctest --test-dir build` 가 Green이 되도록 작성
