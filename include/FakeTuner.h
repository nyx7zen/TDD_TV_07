#pragma once
#include "ITuner.h"
#include <vector>
#include <algorithm>
#include <stdexcept>

class FakeTuner : public ITuner {
    int current_ = 0;
    std::vector<int> available_;
public:
    explicit FakeTuner(std::vector<int> avail)
        : available_(std::move(avail)) {}

    std::string seekCH() override {
        auto it = std::find_if(
            available_.begin(), available_.end(),
            [&](int ch){ return ch > current_; });
        current_ = (it != available_.end())
            ? *it : available_.front();
        return std::to_string(current_);
    }

    void setCH(std::string ch) override {
        int v = std::stoi(ch);
        if (v < 0 || v > 99)
            throw std::invalid_argument("채널 범위 초과: " + ch);
        current_ = v;
    }

    std::string getCurrentCH() override {
        return std::to_string(current_);
    }
};