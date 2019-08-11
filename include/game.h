#ifndef AI_YANG_GAME_H
#define AI_YANG_GAME_H

#include <vector>
#include <set>

#include "util.h"

using namespace std;

class Game : public enable_shared_from_this<Game> {
public:
    explicit Game(shared_ptr<LegStartInfo> &_leg) : leg_info(_leg), round_id(0), is_eat(false), total_score(0.0) {
        power_score = vector<vector<double>>(leg_info->width, vector<double>(leg_info->height, 0.0));
        background_score_limit = vector<vector<double>>(leg_info->width, vector<double>(leg_info->height, 0.0));
        visit_time = vector<vector<int>>(leg_info->width, vector<int>(leg_info->height, 0));
        for (int i = 0; i < leg_info->width; i++) {
            for (int j = 0; j < leg_info->height; j++) {
                Point::Ptr point = leg_info->maps[i][j];
                if (!point->wall && point->tunnel == DIRECTION::NONE && !point->wormhole) {
                    background_score_limit[i][j] = 0.1 / (leg_info->width + leg_info->height);
                }
            }
        }
        background_score = background_score_limit;
    }

    explicit Game(const shared_ptr<Game> &_prev_state) : leg_info(_prev_state->leg_info),
                                                         round_id(_prev_state->round_id),
                                                         is_eat(_prev_state->is_eat),
                                                         total_score(_prev_state->total_score) {
        power_score = _prev_state->power_score;
        background_score = _prev_state->background_score;
        background_score_limit = _prev_state->background_score_limit;
        visit_time = _prev_state->visit_time;
        for (auto &u : _prev_state->my_units) {
            auto &unit = *u.second;
            my_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
        }
        for (auto &u : _prev_state->enemy_units) {
            auto &unit = *u.second;
            enemy_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
        }
    }

    void update_round_info(shared_ptr<RoundInfo> &round_info);

    bool isEnd() { return round_id >= 300; }

    double getAreaScore(Point::Ptr &center, int area);

    Point::Ptr getBestGoal(Point::Ptr &start);

    DIRECTION checkTunnel(int agent_id, DIRECTION action);

    DIRECTION checkRunAway(int agent_id, DIRECTION action);

    bool inVision(Point::Ptr &loc);

public:
    shared_ptr<LegStartInfo> leg_info;
    map<int, Unit::Ptr> my_units;
    map<int, Unit::Ptr> enemy_units;
    int round_id;
    bool is_eat;
    double total_score;
    vector<vector<double>> power_score;
    vector<vector<double>> background_score;
    vector<vector<double>> background_score_limit;
    vector<vector<int>> visit_time;
};

#endif //AI_YANG_GAME_H
