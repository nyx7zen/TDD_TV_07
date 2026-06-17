#include <gtest/gtest.h>
#include "FakeTuner.h"

TEST(FakeTunerTest, setCH_0_최솟값경계_현재채널0) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});

    // When
    tuner.setCH("0");

    // Then
    EXPECT_EQ("0", tuner.getCurrentCH());
}

TEST(FakeTunerTest, setCH_99_최댓값경계_현재채널99) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});

    // When
    tuner.setCH("99");

    // Then
    EXPECT_EQ("99", tuner.getCurrentCH());
}

TEST(FakeTunerTest, setCH_minus1_무효하한_예외발생) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});

    // When / Then
    EXPECT_THROW(tuner.setCH("-1"), std::invalid_argument);
}

TEST(FakeTunerTest, setCH_100_무효상한_예외발생) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});

    // When / Then
    EXPECT_THROW(tuner.setCH("100"), std::invalid_argument);
}

TEST(FakeTunerTest, seekCH_연속호출_항상유효채널반환) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});

    // When / Then
    for (int i = 0; i < 8; ++i) {
        std::string ch = tuner.seekCH();
        int v = std::stoi(ch);
        EXPECT_GE(v, 0);
        EXPECT_LE(v, 99);
    }
}

TEST(FakeTunerTest, seekCH_목록끝_첫채널로wraparound) {
    // Given
    FakeTuner tuner({1, 4, 12, 56});
    tuner.setCH("56");

    // When
    std::string next = tuner.seekCH();

    // Then
    EXPECT_EQ("1", next);
    EXPECT_EQ("1", tuner.getCurrentCH());
}
