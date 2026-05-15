#include <gtest/gtest.h>
#include "FakeTuner.h"

// S: setCH 최솟값 경계
TEST(FakeTunerTest, SetCH_Min_ReturnsZero) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    // When
    tuner.setCH("0");
    // Then
    EXPECT_EQ("0", tuner.getCurrentCH());
}

// S: setCH 최댓값 경계
TEST(FakeTunerTest, SetCH_Max_Returns99) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    // When
    tuner.setCH("99");
    // Then
    EXPECT_EQ("99", tuner.getCurrentCH());
}