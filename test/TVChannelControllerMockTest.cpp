#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TVChannelController.h"
#include "MockTuner.h"

TEST(TVChannelControllerMockTest, Stub) {
    MockTuner mockTuner;
    TVChannelController ctrl(mockTuner);
    EXPECT_TRUE(true);
}