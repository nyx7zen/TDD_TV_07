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
    // Given: 4,5 입력으로 45번 채널 변경 후 6 입력
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
    // Given
    ctrl.pressNumber(9);
    ctrl.pressNumber(9);
    // Then
    EXPECT_EQ("99", tuner.getCurrentCH());
}