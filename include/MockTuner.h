#pragma once
#include <gmock/gmock.h>
#include "ITuner.h"

class MockTuner : public ITuner {
public:
    MOCK_METHOD(std::string, seekCH, (), (override));
    MOCK_METHOD(void, setCH, (std::string ch), (override));
    MOCK_METHOD(std::string, getCurrentCH, (), (override));
};