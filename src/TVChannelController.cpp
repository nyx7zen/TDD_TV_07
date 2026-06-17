#include "TVChannelController.h"
#include <stdexcept>
#include <string>

void TVChannelController::applyChannel(int ch) {
    if (!isValidChannel(ch))
        throw std::invalid_argument("채널 범위 초과: " + std::to_string(ch));
    tuner_.setCH(std::to_string(ch));
}

void TVChannelController::pressNumber(int digit) {
    if (inputBuffer_ == -1) {
        inputBuffer_ = digit;
        return;
    }

    if (inputBuffer_ < 10) {
        int next = inputBuffer_ * 10 + digit;
        if (next > 99)
            throw std::invalid_argument("채널 범위 초과: " + std::to_string(next));
        if (next < 10) {
            applyChannel(next);
            inputBuffer_ = -1;
        } else if (next == 10) {
            inputBuffer_ = 10;
        } else {
            applyChannel(next);
            inputBuffer_ = -1;
        }
        return;
    }

    long next = static_cast<long>(inputBuffer_) * 10 + digit;
    if (next > 99)
        throw std::invalid_argument("채널 범위 초과: " + std::to_string(next));
    applyChannel(static_cast<int>(next));
    inputBuffer_ = -1;
}

void TVChannelController::pressConfirm() {
    if (inputBuffer_ != -1) {
        applyChannel(inputBuffer_);
        inputBuffer_ = -1;
    }
}

void TVChannelController::pressFavorite() {
    int current = std::stoi(tuner_.getCurrentCH());
    if (isFavorite(current)) {
        favorites_.erase(
            std::remove(favorites_.begin(), favorites_.end(), current),
            favorites_.end());
    } else {
        favorites_.push_back(current);
        std::sort(favorites_.begin(), favorites_.end());
    }
}

void TVChannelController::pressNextFavorite() {
    if (favorites_.empty())
        return;

    int current = std::stoi(tuner_.getCurrentCH());
    auto it = std::upper_bound(favorites_.begin(), favorites_.end(), current);
    int next = (it != favorites_.end()) ? *it : favorites_.front();
    applyChannel(next);
}

void TVChannelController::pressOther() {
    inputBuffer_ = -1;
}
