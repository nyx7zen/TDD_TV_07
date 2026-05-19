@README.md @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h @include/ITuner.h

[P] 시니어 C++ 아키텍트 + 모던 C++ 리뷰어입니다.
[C] TDD TV Channel Controller 프로젝트 (C++17, CMake, Google Test / GMock)
[T] TVChannelController(및 연관 코드)를 SOLID / Code Smell 관점에서 분석해줘.
    - SRP/OCP 위반 지점과 근거
      (숫자 입력 버퍼 / 채널 적용 / 선호 채널 토글·이동 / ITuner 위임이 한 클래스에 섞이는지)
    - Magic Number 상수화 필요성
      (채널 0~99, inputBuffer_ 센티넬 -1, 자릿수 10, digit 0~9, FakeTuner 범위 검증 등)
    - Code Smell
      (Long Method, Duplicated Code, 조건문 복잡도, Feature Envy, 컨트롤러·FakeTuner 검증 로직 중복)
    - C++17 스타일 개선 방향
      (책임 분리: ChannelInputBuffer / FavoriteChannelList / applyChannel 파이프라인,
       README dev/refactoring TODO와의 정합성, Command·Handler 등록, std::upper_bound 활용 등)
    - Test Double 설계와의 관계
      (ITuner& DI 유지, Fake vs Mock 검증 경계)
    - 리팩토링 우선순위 (1~5)와 이유
      (Green 테스트 유지 전제, README refactor 항목 반영)
[F] Markdown 표 (문제점 | 위반 원칙/스멜 | 영향 | 개선 방향 | 우선순위)
    + 개선 방향 요약. `docs/code_quality_report.md`에도 저장
