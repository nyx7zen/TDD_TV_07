#include "TVChannelController.h"
#include <stdexcept>

void TVChannelController::pressNumber(int digit) {
    if (inputBuffer_ == -1) {
        inputBuffer_ = digit;
    } else {
        int ch = inputBuffer_ * 10 + digit;
        clearBuffer();
        applyChannel(ch);
    }
}

void TVChannelController::pressConfirm() {
    if (inputBuffer_ != -1) {
        int ch = inputBuffer_;
        clearBuffer();
        applyChannel(ch);
    }
}

void TVChannelController::pressFavorite() {
    int ch = std::stoi(tuner_.getCurrentCH());
    if (isFavorite(ch)) {
        favorites_.erase(
            std::remove(favorites_.begin(), favorites_.end(), ch),
            favorites_.end());
    } else {
        favorites_.push_back(ch);
        std::sort(favorites_.begin(), favorites_.end());
    }
}

void TVChannelController::pressNextFavorite() {
    if (favorites_.empty()) return;

    int cur = std::stoi(tuner_.getCurrentCH());
    auto it = std::upper_bound(
        favorites_.begin(), favorites_.end(), cur);

    int next = (it != favorites_.end())
        ? *it : favorites_.front();

    applyChannel(next);
}

void TVChannelController::pressOther() {
    clearBuffer();
}

void TVChannelController::applyChannel(int ch) {
    if (!isValidChannel(ch))
        throw std::invalid_argument("Invalid channel: " + std::to_string(ch));
    tuner_.setCH(std::to_string(ch));
}