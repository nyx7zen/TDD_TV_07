#pragma once

#include "FakeTuner.h"
#include "TVChannelController.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace golden {

struct ScenarioStep {
    enum class Kind {
        Number,
        Confirm,
        Favorite,
        NextFavorite,
        Other,
        SetChannel,
        AddFavorites
    };

    Kind kind = Kind::Number;
    int digit = 0;
    int channel = 0;
    std::vector<int> favorites;
};

inline std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return {};
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

inline std::vector<int> parseIntList(const std::string& csv) {
    std::vector<int> values;
    std::stringstream ss(csv);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty())
            values.push_back(std::stoi(item));
    }
    return values;
}

inline ScenarioStep::Kind tokenToKind(const std::string& token) {
    if (token == "C" || token == "CONFIRM")
        return ScenarioStep::Kind::Confirm;
    if (token == "F" || token == "FAVORITE")
        return ScenarioStep::Kind::Favorite;
    if (token == "NF" || token == "NEXT_FAVORITE")
        return ScenarioStep::Kind::NextFavorite;
    if (token == "O" || token == "OTHER")
        return ScenarioStep::Kind::Other;
    if (token.size() > 1 && (token[0] == 'N' || token == "NUMBER") &&
        std::isdigit(static_cast<unsigned char>(token[1]))) {
        return ScenarioStep::Kind::Number;
    }
    throw std::runtime_error("unknown scenario token: " + token);
}

inline std::vector<ScenarioStep> loadScenario(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("cannot open scenario: " + path);

    std::vector<ScenarioStep> steps;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.rfind("@CH ", 0) == 0) {
            ScenarioStep step;
            step.kind = ScenarioStep::Kind::SetChannel;
            step.channel = std::stoi(trim(line.substr(4)));
            steps.push_back(step);
            continue;
        }

        if (line.rfind("@FAV ", 0) == 0) {
            ScenarioStep step;
            step.kind = ScenarioStep::Kind::AddFavorites;
            step.favorites = parseIntList(line.substr(5));
            steps.push_back(step);
            continue;
        }

        if (line.rfind("T ", 0) == 0) {
            const int ch = std::stoi(trim(line.substr(2)));
            if (ch < 0 || ch > 99)
                throw std::runtime_error("invalid tune channel: " + line);
            if (ch < 10) {
                ScenarioStep n;
                n.kind = ScenarioStep::Kind::Number;
                n.digit = ch;
                steps.push_back(n);
                ScenarioStep c;
                c.kind = ScenarioStep::Kind::Confirm;
                steps.push_back(c);
            } else {
                ScenarioStep tens;
                tens.kind = ScenarioStep::Kind::Number;
                tens.digit = ch / 10;
                steps.push_back(tens);
                ScenarioStep ones;
                ones.kind = ScenarioStep::Kind::Number;
                ones.digit = ch % 10;
                steps.push_back(ones);
            }
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token.size() == 2 && token[0] == 'N' &&
            std::isdigit(static_cast<unsigned char>(token[1]))) {
            ScenarioStep step;
            step.kind = ScenarioStep::Kind::Number;
            step.digit = token[1] - '0';
            steps.push_back(step);
            continue;
        }

        if (token == "N") {
            int digit = -1;
            iss >> digit;
            if (digit < 0 || digit > 9)
                throw std::runtime_error("invalid digit in: " + line);
            ScenarioStep step;
            step.kind = ScenarioStep::Kind::Number;
            step.digit = digit;
            steps.push_back(step);
            continue;
        }

        ScenarioStep step;
        step.kind = tokenToKind(token);
        steps.push_back(step);
    }

    return steps;
}

inline std::string formatState(FakeTuner& tuner, TVChannelController& ctrl) {
    std::ostringstream os;
    os << "CH=" << tuner.getCurrentCH() << " FAV=[";
    const auto& fav = ctrl.getFavoriteChannels();
    for (size_t i = 0; i < fav.size(); ++i) {
        if (i > 0)
            os << ',';
        os << fav[i];
    }
    os << ']';
    return os.str();
}

inline void applyStep(ScenarioStep step, FakeTuner& tuner,
                      TVChannelController& ctrl) {
    switch (step.kind) {
    case ScenarioStep::Kind::Number:
        ctrl.pressNumber(step.digit);
        break;
    case ScenarioStep::Kind::Confirm:
        ctrl.pressConfirm();
        break;
    case ScenarioStep::Kind::Favorite:
        ctrl.pressFavorite();
        break;
    case ScenarioStep::Kind::NextFavorite:
        ctrl.pressNextFavorite();
        break;
    case ScenarioStep::Kind::Other:
        ctrl.pressOther();
        break;
    case ScenarioStep::Kind::SetChannel:
        tuner.setCH(std::to_string(step.channel));
        break;
    case ScenarioStep::Kind::AddFavorites:
        for (int ch : step.favorites)
            ctrl.addFavorite(ch);
        break;
    }
}

inline std::string runScenario(const std::vector<ScenarioStep>& steps,
                               FakeTuner& tuner, TVChannelController& ctrl,
                               bool expectException) {
    std::ostringstream trace;
    trace << formatState(tuner, ctrl) << '\n';

    try {
        for (const auto& step : steps) {
            applyStep(step, tuner, ctrl);
            trace << formatState(tuner, ctrl) << '\n';
        }
    } catch (const std::exception& ex) {
        if (!expectException)
            throw;
        trace << "!EXCEPTION " << ex.what() << '\n';
    }

    return trace.str();
}

inline std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("cannot open file: " + path);
    std::ostringstream os;
    os << in.rdbuf();
    return os.str();
}

inline void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::trunc);
    if (!out)
        throw std::runtime_error("cannot write file: " + path);
    out << content;
}

inline bool updateGoldensEnabled() {
    const char* env = std::getenv("UPDATE_GOLDENS");
    return env != nullptr && std::string(env) == "1";
}

inline std::string scenarioPath(const std::string& goldenDir,
                                const std::string& name) {
    return goldenDir + "/" + name + ".scenario";
}

inline std::string approvedPath(const std::string& goldenDir,
                                const std::string& name) {
    return goldenDir + "/" + name + ".approved.txt";
}

}  // namespace golden
