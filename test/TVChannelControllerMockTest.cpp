#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TVChannelController.h"
#include "MockTuner.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

class MockControllerTest : public ::testing::Test {
protected:
    MockTuner mockTuner;
    TVChannelController ctrl{mockTuner};
};

TEST_F(MockControllerTest, 한자리확정_setCH_1회호출) {
    // Given
    EXPECT_CALL(mockTuner, setCH("1")).Times(1);

    // When
    ctrl.pressNumber(1);
    ctrl.pressConfirm();

    // Then — EXPECT_CALL
}

TEST_F(MockControllerTest, 두자리입력_setCH_12_1회호출) {
    // Given
    EXPECT_CALL(mockTuner, setCH("12")).Times(1);

    // When
    ctrl.pressNumber(1);
    ctrl.pressNumber(2);

    // Then — EXPECT_CALL
}

TEST_F(MockControllerTest, 즐겨찾기토글_getCurrentCH_호출됨) {
    // Given
    EXPECT_CALL(mockTuner, getCurrentCH())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return("5"));

    // When
    ctrl.pressFavorite();

    // Then — EXPECT_CALL
}

TEST_F(MockControllerTest, 다음선호_목록있음_getCurrentCH후setCH_순서검증) {
    // Given
    ctrl.addFavorite(1);
    ctrl.addFavorite(4);
    ctrl.addFavorite(12);
    ctrl.addFavorite(56);

    {
        InSequence seq;
        EXPECT_CALL(mockTuner, getCurrentCH()).WillOnce(Return("6"));
        EXPECT_CALL(mockTuner, setCH("12")).Times(1);
    }

    // When
    ctrl.pressNextFavorite();

    // Then — InSequence
}

TEST_F(MockControllerTest, 빈즐겨찾기_다음선호_setCH_미호출) {
    // Given
    EXPECT_CALL(mockTuner, setCH(_)).Times(0);

    // When
    ctrl.pressNextFavorite();

    // Then — setCH 미호출
}
