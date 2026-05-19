#include <gtest/gtest.h>

#include "golden_master/ScenarioRunner.h"

#include <cstdlib>
#include <string>
#include <vector>

#ifndef GOLDEN_DIR
#define GOLDEN_DIR "test/golden"
#endif

namespace {

class GoldenMasterTest : public ::testing::Test {
protected:
    FakeTuner tuner{std::vector<int>{1, 4, 12, 56}};
    TVChannelController ctrl{tuner};

    void SetUp() override { tuner.setCH("0"); }

    void runGoldenScenario(const std::string& name, bool expectException = false) {
        const std::string goldenDir = GOLDEN_DIR;
        const auto scenarioFile = golden::scenarioPath(goldenDir, name);
        const auto approvedFile = golden::approvedPath(goldenDir, name);

        const auto steps = golden::loadScenario(scenarioFile);
        const std::string actual =
            golden::runScenario(steps, tuner, ctrl, expectException);

        if (golden::updateGoldensEnabled()) {
            golden::writeFile(approvedFile, actual);
            GTEST_SKIP() << "Updated golden file: " << approvedFile;
        }

        const std::string expected = golden::readFile(approvedFile);
        EXPECT_EQ(expected, actual)
            << "Golden mismatch for " << name
            << "\n  scenario: " << scenarioFile
            << "\n  approved: " << approvedFile
            << "\nSet UPDATE_GOLDENS=1 to refresh approved files.";
    }
};

}  // namespace

TEST_F(GoldenMasterTest, ChannelSingleConfirm) {
    runGoldenScenario("channel_single_confirm");
}

TEST_F(GoldenMasterTest, ChannelTwoDigitImmediate) {
    runGoldenScenario("channel_two_digit_immediate");
}

TEST_F(GoldenMasterTest, ChannelSequence12Then34) {
    runGoldenScenario("channel_sequence_12_then_34");
}

TEST_F(GoldenMasterTest, ChannelBufferClearedByOther) {
    runGoldenScenario("channel_buffer_cleared_by_other");
}

TEST_F(GoldenMasterTest, ChannelLeadingZero07) {
    runGoldenScenario("channel_leading_zero_07");
}

TEST_F(GoldenMasterTest, Channel99Boundary) {
    runGoldenScenario("channel_99_boundary");
}

TEST_F(GoldenMasterTest, FavoritesComplexToggle) {
    runGoldenScenario("favorites_complex_toggle");
}

TEST_F(GoldenMasterTest, NextFavoriteFrom6) {
    runGoldenScenario("next_favorite_from_6");
}

TEST_F(GoldenMasterTest, NextFavoriteWrap56To1) {
    runGoldenScenario("next_favorite_wrap_56_to_1");
}

TEST_F(GoldenMasterTest, NextFavoriteEmptyList) {
    runGoldenScenario("next_favorite_empty_list");
}

TEST_F(GoldenMasterTest, ExceptionChannel100) {
    runGoldenScenario("exception_channel_100", true);
}

TEST_F(GoldenMasterTest, ReadmeSmokeEndToEnd) {
    runGoldenScenario("readme_smoke_end_to_end");
}
