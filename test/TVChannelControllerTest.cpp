#include <gtest/gtest.h>
#include "TVChannelController.h"
#include "FakeTuner.h"

class ControllerTest : public ::testing::Test {
protected:
    FakeTuner tuner{std::vector<int>{1, 4, 12, 56}};
    TVChannelController ctrl{tuner};

    void SetUp() override { tuner.setCH("0"); }

    void tuneTo(int ch) {
        if (ch < 10) {
            ctrl.pressNumber(ch);
            ctrl.pressConfirm();
        } else {
            ctrl.pressNumber(ch / 10);
            ctrl.pressNumber(ch % 10);
        }
    }
};

// --- 기능 1: 숫자 입력·확정 ---

TEST_F(ControllerTest, 한자리입력_확정_채널1로변경) {
    // Given — 초기 채널 "0"

    // When
    ctrl.pressNumber(1);
    ctrl.pressConfirm();

    // Then
    EXPECT_EQ("1", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 두자리입력_즉시적용_채널12로변경) {
    // Given

    // When
    ctrl.pressNumber(1);
    ctrl.pressNumber(2);

    // Then
    EXPECT_EQ("12", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 연속입력_두번의두자리_12후34로변경) {
    // Given

    // When
    ctrl.pressNumber(1);
    ctrl.pressNumber(2);
    ctrl.pressNumber(3);
    ctrl.pressNumber(4);

    // Then
    EXPECT_EQ("34", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 버퍼입력후Other_45유지_6미적용) {
    // Given

    // When
    ctrl.pressNumber(4);
    ctrl.pressNumber(5);
    ctrl.pressNumber(6);
    ctrl.pressOther();

    // Then
    EXPECT_EQ("45", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 선행0입력_07_채널7로변경) {
    // Given

    // When
    ctrl.pressNumber(0);
    ctrl.pressNumber(7);

    // Then
    EXPECT_EQ("7", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 채널0입력_확정_채널0유지) {
    // Given — 초기 "0"

    // When
    ctrl.pressNumber(0);
    ctrl.pressConfirm();

    // Then
    EXPECT_EQ("0", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 채널99입력_두자리_채널99로변경) {
    // Given

    // When
    ctrl.pressNumber(9);
    ctrl.pressNumber(9);

    // Then
    EXPECT_EQ("99", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 채널100이상_입력_예외발생) {
    // Given

    // When / Then
    EXPECT_THROW(
        {
            ctrl.pressNumber(1);
            ctrl.pressNumber(0);
            ctrl.pressNumber(0);
        },
        std::invalid_argument);
}

// --- 기능 2: 선호 채널 토글 ---

TEST_F(ControllerTest, 미등록채널_즐겨찾기_목록에추가) {
    // Given
    tuneTo(5);

    // When
    ctrl.pressFavorite();

    // Then
    const auto& fav = ctrl.getFavoriteChannels();
    ASSERT_EQ(1u, fav.size());
    EXPECT_EQ(5, fav[0]);
}

TEST_F(ControllerTest, 등록채널_즐겨찾기_목록에서삭제) {
    // Given
    tuneTo(5);
    ctrl.pressFavorite();

    // When
    ctrl.pressFavorite();

    // Then
    EXPECT_TRUE(ctrl.getFavoriteChannels().empty());
}

TEST_F(ControllerTest, 복합토글시퀀스_목록637오름차순) {
    // Given / When — 12→★ 08→★ 37→★ 08→★ 06→★
    tuneTo(12);
    ctrl.pressFavorite();
    tuneTo(8);
    ctrl.pressFavorite();
    tuneTo(37);
    ctrl.pressFavorite();
    tuneTo(8);
    ctrl.pressFavorite();
    tuneTo(6);
    ctrl.pressFavorite();

    // Then
    const std::vector<int> expected{6, 12, 37};
    EXPECT_EQ(expected, ctrl.getFavoriteChannels());
}

TEST_F(ControllerTest, 즐겨찾기목록_여러추가_오름차순정렬) {
    // Given
    ctrl.addFavorite(30);
    ctrl.addFavorite(5);
    ctrl.addFavorite(15);

    // When / Then
    const std::vector<int> expected{5, 15, 30};
    EXPECT_EQ(expected, ctrl.getFavoriteChannels());
}

// --- 기능 3: 다음 선호 채널 ---

TEST_F(ControllerTest, 다음선호_현재6_채널12로이동) {
    // Given
    ctrl.addFavorite(1);
    ctrl.addFavorite(4);
    ctrl.addFavorite(12);
    ctrl.addFavorite(56);
    tuner.setCH("6");

    // When
    ctrl.pressNextFavorite();

    // Then
    EXPECT_EQ("12", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 다음선호_현재56_채널1로wraparound) {
    // Given
    ctrl.addFavorite(1);
    ctrl.addFavorite(4);
    ctrl.addFavorite(12);
    ctrl.addFavorite(56);
    tuner.setCH("56");

    // When
    ctrl.pressNextFavorite();

    // Then
    EXPECT_EQ("1", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 빈즐겨찾기_다음선호_채널변화없음) {
    // Given
    tuner.setCH("6");

    // When
    ctrl.pressNextFavorite();

    // Then
    EXPECT_EQ("6", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 목록외현재채널_다음선호_첫큰값으로이동) {
    // Given
    ctrl.addFavorite(10);
    ctrl.addFavorite(20);
    tuner.setCH("5");

    // When
    ctrl.pressNextFavorite();

    // Then
    EXPECT_EQ("10", tuner.getCurrentCH());
}

TEST_F(ControllerTest, 단일즐겨찾기_다음선호_wraparound동일채널) {
    // Given
    ctrl.addFavorite(7);
    tuner.setCH("7");

    // When
    ctrl.pressNextFavorite();

    // Then
    EXPECT_EQ("7", tuner.getCurrentCH());
}
