@README.md @tasks/step-1_requirements-analysis.md @include/TVChannelController.h @src/TVChannelController.cpp @include/FakeTuner.h @test/TVChannelControllerTest.cpp @CMakeLists.txt

[P] 회귀 테스트(Approval / Golden Master) 설계 전문가입니다.
[C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake)
[T] README 시나리오 기반 Golden Master 회귀 테스트를 설계·구현해줘.
    1) 시나리오 스크립트 → 텍스트 스냅샷 출력 형식 정의
       (예: pressNumber/pressConfirm/pressFavorite/pressNextFavorite/pressOther 시퀀스,
            각 단계 후 `CH=<현재채널> FAV=[...]` 한 줄 덤프)
    2) expected 출력 파일 생성·보관 전략
       (`test/golden/` 또는 `approval/`, 시나리오별 `.approved.txt`, 갱신 절차 문서화)
    3) Google Test에서 파일 비교 테스트 구현
       (예: `TVChannelGoldenTest.cpp`, FakeTuner 사용, 전체 출력 문자열 vs golden 파일)
    4) CMake / ctest 통합
       (새 executable `golden_test` 등록, `gtest_discover_tests` 포함)
    5) CI에서 자동 실행 구성
       (예: GitHub Actions workflow 또는 로컬 `ctest` 단계 설명)
    - 단, `ITuner.h` 수정 금지
[F] 테스트 코드 + CMake 수정안 + golden 파일 샘플 + 실행·갱신 방법 (Markdown 요약 포함).
     `docs/golden_master_guide.md`에도 저장
