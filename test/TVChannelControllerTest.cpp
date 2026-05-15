#include <gtest/gtest.h>
#include "TVChannelController.h"
#include "FakeTuner.h"

TEST(TVChannelControllerTest, Stub) {
    FakeTuner tuner({1, 4, 12, 56});
    TVChannelController ctrl(tuner);
    EXPECT_EQ("0", tuner.getCurrentCH());
}