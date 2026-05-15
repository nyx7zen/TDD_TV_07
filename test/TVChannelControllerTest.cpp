#include <gtest/gtest.h>
#include "TVChannelController.h"
#include "FakeTuner.h"

class ControllerTest : public ::testing::Test {
protected:
    FakeTuner tuner{std::vector<int>{1, 4, 12, 56}};
    TVChannelController ctrl{tuner};
};

// S1-1: 한 자리 입력 + 확인
TEST_F(ControllerTest, PressNumber_SingleDigit_ChannelChanges) {
    // Given
    ctrl.pressNumber(1);
    // When
    ctrl.pressConfirm();
    // Then
    EXPECT_EQ("1", tuner.getCurrentCH());
}

// S1-2: 두 자리 자동 변경
TEST_F(ControllerTest, PressNumber_TwoDigits_AutoChanges) {
    // Given / When
    ctrl.pressNumber(1);
    ctrl.pressNumber(2);
    // Then
    EXPECT_EQ("12", tuner.getCurrentCH());
}

// S1-3: 연속 네 자리 입력
TEST_F(ControllerTest, PressNumber_FourDigits_TwoStageChanges) {
    // Given / When
    ctrl.pressNumber(1);
    ctrl.pressNumber(2);
    ctrl.pressNumber(3);
    ctrl.pressNumber(4);
    // Then
    EXPECT_EQ("34", tuner.getCurrentCH());
}

// S1-4: 버퍼 무효화
TEST_F(ControllerTest, PressOther_CancelsBuffer_NoChannelChange) {
    // Given
    ctrl.pressNumber(4);
    ctrl.pressNumber(5);  // 45번으로 자동 변경
    ctrl.pressNumber(6);  // 6이 버퍼에 저장
    // When
    ctrl.pressOther();    // 6 무효화
    // Then: 45번 유지
    EXPECT_EQ("45", tuner.getCurrentCH());
}

// S1-5: 0으로 시작하는 입력
TEST_F(ControllerTest, PressNumber_StartsWithZero_SecondDigitOnly) {
    // Given / When
    ctrl.pressNumber(0);
    ctrl.pressNumber(7);
    // Then
    EXPECT_EQ("7", tuner.getCurrentCH());
}

// 경계값: 채널 0
TEST_F(ControllerTest, PressNumber_ChannelZero_ChangesNormally) {
    // Given
    ctrl.pressNumber(0);
    // When
    ctrl.pressConfirm();
    // Then
    EXPECT_EQ("0", tuner.getCurrentCH());
}

// 경계값: 채널 99
TEST_F(ControllerTest, PressNumber_Channel99_ChangesNormally) {
    // Given / When
    ctrl.pressNumber(9);
    ctrl.pressNumber(9);
    // Then
    EXPECT_EQ("99", tuner.getCurrentCH());
}

// S2-1: 미등록 채널 추가
TEST_F(ControllerTest, PressFavorite_NewChannel_AddedToList) {
    // Given
    tuner.setCH("12");
    // When
    ctrl.pressFavorite();
    // Then
    const auto& favs = ctrl.getFavoriteChannels();
    ASSERT_FALSE(favs.empty());
    EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

// S2-2: 등록된 채널 삭제 (토글)
TEST_F(ControllerTest, PressFavorite_ExistingChannel_RemovedFromList) {
    // Given
    tuner.setCH("12");
    ctrl.pressFavorite();
    // When
    ctrl.pressFavorite();
    // Then
    const auto& favs = ctrl.getFavoriteChannels();
    EXPECT_EQ(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

// S2-3: 토글 전체 시나리오
TEST_F(ControllerTest, PressFavorite_ToggleScenario_ResultList) {
    // Given / When
    for (int ch : {12, 8, 37, 8, 6}) {
        tuner.setCH(std::to_string(ch));
        ctrl.pressFavorite();
    }
    // Then: {6, 12, 37} 만 남아야 함
    const auto& favs = ctrl.getFavoriteChannels();
    ASSERT_EQ(3u, favs.size());
    EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 6));
    EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
    EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 37));
}

// S2: 목록 정렬 검증
TEST_F(ControllerTest, PressFavorite_MultipleChannels_ListIsSorted) {
    // Given / When
    for (int ch : {37, 6, 12}) {
        tuner.setCH(std::to_string(ch));
        ctrl.pressFavorite();
    }
    // Then
    const auto& favs = ctrl.getFavoriteChannels();
    ASSERT_FALSE(favs.empty());
    EXPECT_TRUE(std::is_sorted(favs.begin(), favs.end()));
}