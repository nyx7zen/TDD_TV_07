#pragma once
#include "ITuner.h"
#include <vector>
#include <algorithm>
#include <string>

class TVChannelController {
    ITuner& tuner_;
    int inputBuffer_ = -1;
    std::vector<int> favorites_;

    bool isValidChannel(int ch) const {
        return ch >= 0 && ch <= 99;
    }

    bool isFavorite(int ch) const {
        return std::find(favorites_.begin(),
            favorites_.end(), ch) != favorites_.end();
    }

    void clearBuffer() {
        inputBuffer_ = -1;
    }

    void applyChannel(int ch);

    void addToFavorites(int ch) {
        if (!isFavorite(ch)) {
            favorites_.push_back(ch);
            std::sort(favorites_.begin(), favorites_.end());
        }
    }

    int findNextFavorite(int cur) const {
        auto it = std::upper_bound(
            favorites_.begin(), favorites_.end(), cur);
        return (it != favorites_.end())
            ? *it : favorites_.front();
    }

public:
    explicit TVChannelController(ITuner& t) : tuner_(t) {}

    void pressNumber(int digit);
    void pressConfirm();
    void pressFavorite();
    void pressNextFavorite();
    void pressOther();

    const std::vector<int>& getFavoriteChannels() const {
        return favorites_;
    }

    void addFavorite(int ch) {
        addToFavorites(ch);
    }
};