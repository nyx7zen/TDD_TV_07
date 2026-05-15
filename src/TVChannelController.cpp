#include "TVChannelController.h"
#include <stdexcept>

void TVChannelController::pressNumber(int digit) {
    if (inputBuffer_ == -1) {
        inputBuffer_ = digit;
    } else {
        int ch = inputBuffer_ * 10 + digit;
        inputBuffer_ = -1;
        applyChannel(ch);
    }
}

void TVChannelController::pressConfirm() {
    if (inputBuffer_ != -1) {
        int ch = inputBuffer_;
        inputBuffer_ = -1;
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
}

void TVChannelController::pressOther() {
    inputBuffer_ = -1;
}

void TVChannelController::applyChannel(int ch) {
    if (!isValidChannel(ch))
        throw std::invalid_argument("Invalid channel: " + std::to_string(ch));
    tuner_.setCH(std::to_string(ch));
}