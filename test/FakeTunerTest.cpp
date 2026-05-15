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

// setCH 범위 초과 예외
TEST(FakeTunerTest, SetCH_Negative_ThrowsException) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    // When / Then
    EXPECT_THROW(tuner.setCH("-1"), std::invalid_argument);
}

TEST(FakeTunerTest, SetCH_Over99_ThrowsException) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    // When / Then
    EXPECT_THROW(tuner.setCH("100"), std::invalid_argument);
}

// seekCH wrap-around
TEST(FakeTunerTest, SeekCH_WrapAround_ReturnsFirst) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    tuner.setCH("56");
    // When
    std::string result = tuner.seekCH();
    // Then
    EXPECT_EQ("1", result);
}

// seekCH 연속 호출
TEST(FakeTunerTest, SeekCH_Sequential_ReturnsValidChannel) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    // When / Then
    EXPECT_EQ("1", tuner.seekCH());
    EXPECT_EQ("4", tuner.seekCH());
    EXPECT_EQ("12", tuner.seekCH());
    EXPECT_EQ("56", tuner.seekCH());
}