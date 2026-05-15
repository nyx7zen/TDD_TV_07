#define APPROVALS_GOOGLETEST
#include "ApprovalTests.hpp"
#include <gtest/gtest.h>
#include "TVChannelController.h"
#include "FakeTuner.h"
#include <fstream>
#include <sstream>

static const std::string APPROVED_FILE =
    "C:/DEV/week1_day5/TDD_TV_07/test/ApprovalTest.approved.txt";

std::string runScenario() {
    std::ostringstream oss;

    // 기능 1: 숫자 버튼 채널 변경
    oss << "=== 기능 1: 숫자 버튼 채널 변경 ===\n";

    FakeTuner tuner1({1, 4, 12, 56});
    TVChannelController ctrl1(tuner1);

    ctrl1.pressNumber(1);
    ctrl1.pressConfirm();
    oss << "pressNumber(1) + pressConfirm() -> " << tuner1.getCurrentCH() << "\n";

    ctrl1.pressNumber(1);
    ctrl1.pressNumber(2);
    oss << "pressNumber(1,2) -> " << tuner1.getCurrentCH() << "\n";

    ctrl1.pressNumber(1);
    ctrl1.pressNumber(2);
    ctrl1.pressNumber(3);
    ctrl1.pressNumber(4);
    oss << "pressNumber(1,2,3,4) -> " << tuner1.getCurrentCH() << "\n";

    ctrl1.pressNumber(4);
    ctrl1.pressNumber(5);
    ctrl1.pressNumber(6);
    ctrl1.pressOther();
    oss << "pressNumber(4,5,6) + pressOther() -> " << tuner1.getCurrentCH() << "\n";

    ctrl1.pressNumber(0);
    ctrl1.pressNumber(7);
    oss << "pressNumber(0,7) -> " << tuner1.getCurrentCH() << "\n";

    // 기능 2: 선호 채널 토글
    oss << "\n=== 기능 2: 선호 채널 토글 ===\n";

    FakeTuner tuner2({1, 4, 12, 56});
    TVChannelController ctrl2(tuner2);

    for (int ch : {12, 8, 37, 8, 6}) {
        tuner2.setCH(std::to_string(ch));
        ctrl2.pressFavorite();
        oss << "pressFavorite() ch=" << ch
            << " -> 목록: {";
        const auto& favs = ctrl2.getFavoriteChannels();
        for (size_t i = 0; i < favs.size(); i++) {
            oss << favs[i];
            if (i + 1 < favs.size()) oss << ",";
        }
        oss << "}\n";
    }

    // 기능 3: 다음 선호 채널
    oss << "\n=== 기능 3: 다음 선호 채널 ===\n";

FakeTuner tuner3({1, 4, 12, 56});
    TVChannelController ctrl3(tuner3);
    for (int ch : {1, 4, 12, 56}) ctrl3.addFavorite(ch);

    tuner3.setCH("6");
    ctrl3.pressNextFavorite();
    oss << "현재 6 -> pressNextFavorite() -> " << tuner3.getCurrentCH() << "\n";

    tuner3.setCH("56");
    ctrl3.pressNextFavorite();
    oss << "현재 56 -> pressNextFavorite() -> " << tuner3.getCurrentCH() << " (wrap-around)\n";

    FakeTuner tuner4({1, 4, 12, 56});
    TVChannelController ctrl4(tuner4);
    tuner4.setCH("6");
    ctrl4.pressNextFavorite();
    oss << "빈 목록 -> pressNextFavorite() -> " << tuner4.getCurrentCH() << " (변화 없음)\n";

    return oss.str();
}

TEST(ApprovalTest, GenerateApprovedFile) {
    std::string result = runScenario();

    std::ofstream out(APPROVED_FILE);
    out << result;
    out.close();

    std::cout << result;
    SUCCEED();
}

TEST(ApprovalTest, VerifyApprovedFile) {
    std::ifstream in(APPROVED_FILE);
    ASSERT_TRUE(in.is_open()) << "ApprovalTest.approved.txt 파일이 없습니다.";

    std::string expected((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
    in.close();

    std::string actual = runScenario();
    EXPECT_EQ(expected, actual);
}