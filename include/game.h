#ifndef AI_YANG_GAME_H
#define AI_YANG_GAME_H

#include <vector>
#include <set>

#include "util.h"

using namespace std;

class Game : public enable_shared_from_this<Game> {
public:
    explicit Game(shared_ptr<LegStartInfo> &_leg) : leg_info(_leg), round_id(0), is_eat(false) {
        power_score = vector<vector<double>>(leg_info->width, vector<double>(leg_info->height, 0.0));
        env_score_limit = vector<vector<double>>(leg_info->width, vector<double>(leg_info->height, 0.0));
        visit_time = vector<vector<int>>(leg_info->width, vector<int>(leg_info->height, 0));
        for (int i = 0; i < leg_info->width; i++) {
            for (int j = 0; j < leg_info->height; j++) {
                Point::Ptr point = leg_info->maps[i][j];
                if (!point->wall && point->tunnel == DIRECTION::NONE && !point->wormhole) {
                    env_score_limit[i][j] = 1.0 / (my_pow(2 * leg_info->vision + 1, 2));
                }
            }
        }
        env_score = env_score_limit;
    }

    void update_round_info(shared_ptr<RoundInfo> &round_info);

    bool isEnd() { return round_id >= 300; }

public:
    shared_ptr<LegStartInfo> leg_info;
    map<int, Unit::Ptr> my_units;
    map<int, Unit::Ptr> enemy_units;
    int round_id;
    bool is_eat;
    vector<vector<double>> power_score;
    vector<vector<double>> env_score;
    vector<vector<double>> env_score_limit;
    vector<vector<int>> visit_time;
};

#endif //AI_YANG_GAME_H
