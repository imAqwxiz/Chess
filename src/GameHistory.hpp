#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

struct GameRecord {
    std::string username;      // opponent
    int elo = 1200;            // opponent elo
    std::string result;        // "Win", "Loss" or "Draw"
    int eloChange = 0;         // + gained, - lost
    int64_t date = 0;          // unix time
};

template <>
struct matjson::Serialize<GameRecord> {
    static geode::Result<GameRecord> fromJson(matjson::Value const& v) {
        GameRecord r;
        r.username  = v["username"].asString().unwrapOr("Unknown");
        r.elo       = static_cast<int>(v["elo"].asInt().unwrapOr(1200));
        r.result    = v["result"].asString().unwrapOr("Loss");
        r.eloChange = static_cast<int>(v["eloChange"].asInt().unwrapOr(0));
        r.date      = v["date"].asInt().unwrapOr(0);
        return geode::Ok(r);
    }

    static matjson::Value toJson(GameRecord const& r) {
        return matjson::makeObject({
            {"username", r.username},
            {"elo", r.elo},
            {"result", r.result},
            {"eloChange", r.eloChange},
            {"date", r.date}
        });
    }
};

namespace GameHistory {
    inline std::vector<GameRecord> load() {
        return geode::Mod::get()->getSavedValue<std::vector<GameRecord>>("games", {});
    }

    // Call this when a game ends (keeps the last 50)
    inline void add(GameRecord const& r) {
        auto games = load();
        games.push_back(r);
        if (games.size() > 50) {
            games.erase(games.begin(), games.begin() + (games.size() - 50));
        }
        geode::Mod::get()->setSavedValue("games", games);
    }
}
