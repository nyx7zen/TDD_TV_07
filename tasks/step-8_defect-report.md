@README.md @tasks/step-1_requirements-analysis.md @tasks/step-3_test-plan.md @tasks/step-5B_defect-list.md @Report/defect_list.md @Report/requirements_analysis.md @Report/test_plan.md @docs/defect_list.md

[P] QA 리드 엔지니어입니다.
[C] TDD TV Channel Controller (C++17, Google Test / GMock, CMake, gcov/lcov)
[T] 결함 관리 문서를 작성해줘.
    1) 결함 분류 체계:
       Severity (Critical/Major/Minor/Info) × Area (5종) 매트릭스 표
       - Area: FakeTuner / 기능1(숫자입력) / 기능2(선호토글) / 기능3(다음선호) / Mock검증
    2) 결함 보고서 템플릿 (재현/기대/실제/원인/수정/검증)
       - `defect_list.md` 항목과 ID·Severity 연계
    3) 품질 메트릭 수집 계획
       - 테스트 통과율 (fake_tuner_test / controller_test / controller_mock_test / golden_test)
       - 커버리지 (README 목표 80%+, gcov/lcov, `tv_controller` 라이브러리 기준)
       - 단계별 결함 발견율 (step-4 구현 → step-5 디버깅 → step-7 리팩토링)
    4) (선택) GitHub Issues 연동 워크플로우
       (라벨: severity/area, PR·ctest 실패 연계)
[F] Markdown. `docs/defect_report.md`에도 저장
