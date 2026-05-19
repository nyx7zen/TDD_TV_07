@README.md @tasks/step-1_requirements-analysis.md @tasks/step-5_debug-test-failures.md @Report/defect_list.md @docs/defect_list.md @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h @test/FakeTunerTest.cpp @test/TVChannelControllerTest.cpp @test/TVChannelControllerMockTest.cpp

[P] QA 리드입니다.
[C] TDD TV Channel Controller — 현재까지 발견된 테스트 실패·결함 문서화
[T] 발견된 결함을 다음 형식으로 정리해줘.
    항목: [ID] [Severity] [Area] [Steps] [Expected] [Actual] [Root Cause] [Fix Summary]
    - [Area]: FakeTuner / 기능1(숫자입력) / 기능2(선호토글) / 기능3(다음선호) / Mock검증 / Controller공통
    - ctest 실패 로그·step-5 분석 결과가 있으면 반영; 없으면 README 시나리오 대비 현재 구현 갭 기준으로 작성
[F] Markdown. `docs/defect_list.md`에도 저장
