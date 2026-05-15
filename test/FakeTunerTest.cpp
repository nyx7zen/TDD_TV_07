#include <gtest/gtest.h>
#include "FakeTuner.h"

TEST(FakeTunerTest, Stub) {
    FakeTuner tuner({1, 4, 12, 56});
    EXPECT_EQ("0", tuner.getCurrentCH());
}