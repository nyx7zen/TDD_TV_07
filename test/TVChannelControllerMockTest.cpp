#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TVChannelController.h"
#include "MockTuner.h"

using ::testing::Return;
using ::testing::_;

class ControllerMockTest : public ::testing::Test {
protected:
    MockTuner mockTuner;
    std::unique_ptr<TVChannelController> ctrl;

    void SetUp() override {
        ctrl = std::make_unique<TVChannelController>(mockTuner);
    }
};

// pressNumber(1) + pressConfirm() → setCH("1") 1회 호출
TEST_F(ControllerMockTest, PressNumber1Confirm_CallsSetCH1) {
    // Given / When / Then
    EXPECT_CALL(mockTuner, setCH("1")).Times(1);
    ctrl->pressNumber(1);
    ctrl->pressConfirm();
}

// pressNumber(1) + pressNumber(2) → setCH("12") 1회 호출
TEST_F(ControllerMockTest, Press1Then2_CallsSetCH12) {
    // Given / When / Then
    EXPECT_CALL(mockTuner, setCH("12")).Times(1);
    ctrl->pressNumber(1);
    ctrl->pressNumber(2);
}

// pressFavorite() → getCurrentCH() 호출 검증
TEST_F(ControllerMockTest, PressFavorite_CallsGetCurrentCH) {
    // Given / When / Then
    EXPECT_CALL(mockTuner, getCurrentCH())
        .WillOnce(Return("12"));
    ctrl->pressFavorite();
}

// pressNextFavorite() (목록 있을 때) → getCurrentCH() 후 setCH() 호출
TEST_F(ControllerMockTest, PressNextFavorite_CallsGetCurrentThenSetCH) {
    // Given
    ctrl->addFavorite(12);
    ctrl->addFavorite(56);
    EXPECT_CALL(mockTuner, getCurrentCH())
        .WillOnce(Return("6"));
    EXPECT_CALL(mockTuner, setCH("12")).Times(1);
    // When
    ctrl->pressNextFavorite();
}

// pressNextFavorite() (목록 비어있을 때) → setCH() 호출 없음
TEST_F(ControllerMockTest, PressNextFavorite_EmptyList_NeverCallsSetCH) {
    // Given / When / Then
    EXPECT_CALL(mockTuner, setCH(_)).Times(0);
    ctrl->pressNextFavorite();
}