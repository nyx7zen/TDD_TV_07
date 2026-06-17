@README.md @tasks/step-1_requirements-analysis.md @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h @include/MockTuner.h @test/FakeTunerTest.cpp @test/TVChannelControllerTest.cpp @test/TVChannelControllerMockTest.cpp @CMakeLists.txt

[P] 시니어 QA 리드입니다.
[C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake, gcov/lcov 가능 시 포함)
[T] 테스트 계획서를 작성해줘.
    - TEST / TEST_F 기반 단위 테스트 범위·우선순위
      (fake_tuner_test → controller_test → controller_mock_test, TDD Red→Green 순서)
    - 경계값 케이스 목록
      (채널 0, 1, 98, 99 / -1, 100 / digit 0~9 / inputBuffer_ -1 / 빈·단일 favorites_ / wrap-around)
    - 예외·특이 케이스 목록
      (std::invalid_argument, pressOther 버퍼 무효화, 목록 외 현재 채널, Mock 미호출 등)
    - Fake vs Mock 검증 분리 전략
      (상태 기반 vs 행위 기반, EXPECT_CALL·InSequence)
    - 커버리지 목표 (README 기준 80%+, 권장 90%+)와 gcov/lcov 측정·개선 전략
    - README TO-DO 28개 시나리오와의 추적 매트릭스 (번호·테스트명·우선순위)
[F] Markdown 문서. `docs/test_plan.md`에도 저장
