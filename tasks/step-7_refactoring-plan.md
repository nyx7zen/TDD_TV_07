@README.md @tasks/step-1_requirements-analysis.md @tasks/step-2_code-quality.md @Report/code_quality_report.md @Report/requirements_analysis.md @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h

[P] 모던 C++ 리팩토링 코치입니다.
[C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
[T] TVChannelController 리팩토링 계획을 단계별로 제안해줘.
    제약:
    - `ITuner.h` 수정 금지
    - 채널 유효 범위 0~99 유지
    - 모든 단계에서 테스트 Green 유지 (`dev/refactoring` 브랜치, README refactor TODO 준수)
    내용:
    - 조건 분기 축소·중복 제거 (커밋 단위로 쪼개기)
      (`clearBuffer`, `findNextFavorite`, `addToFavorites`, `applyChannel` 파이프라인 등)
    - 타입·책임 분리
      (ChannelInputBuffer / FavoriteChannelList / 버튼 Command·Handler, `std::upper_bound` 활용)
    - 매직 넘버 상수화와 C++17 스타일 개선
      (-1, 0, 99, 10, digit 범위; 컨트롤러·FakeTuner 검증 중복 정리)
    - Fake vs Mock 테스트 회귀 영향 점검
[F] 단계별 체크리스트 + 각 단계 검증 방법 (`cmake --build build && ctest --test-dir build`).
    `docs/refactoring_plan.md`에도 저장
