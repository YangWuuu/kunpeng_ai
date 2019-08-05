#ifndef AI_YANG_GAME_H
#define AI_YANG_GAME_H

#include <vector>

#include "util.h"

using namespace std;

class GameState {
public:
    GameState(LegStartInfo *_leg, RoundInfo *_ri) : leg(_leg), round_id(0), isEat(false), total_score(0.0) {
        power_score = vector<vector<double>>(leg->width, vector<double>(leg->height, 0.0));
        background_score_limit = vector<vector<double>>(leg->width, vector<double>(leg->height, 0.0));
        visit_time = vector<vector<int>>(leg->width, vector<int>(leg->height, 0));
        for (int i = 0; i < leg->width; i++) {
            for (int j = 0; j < leg->height; j++) {
                Point::Ptr point = leg->maps[i][j];
                if (!point->wall && point->tunnel == Direction::NONE && !point->wormhole) {
                    background_score_limit[i][j] = 1.0 / (leg->width + leg->height);
                }
            }
        }
        background_score = background_score_limit;
    }

    explicit GameState(GameState *_prev_state) : leg(_prev_state->leg), round_id(_prev_state->round_id),
                                                 isEat(_prev_state->isEat), total_score(_prev_state->total_score) {
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

    void updateRoundInfo(RoundInfo &ri);

    bool isEnd() { return round_id >= 300; }

    vector<Direction> getLegalActions(int agent_id);

    GameState *generateSuccessor(int agent_id, Direction action);

    GameState *generateEnemySuccessor(int agent_id, Direction action);

public:
    LegStartInfo *leg;
    map<int, Unit::Ptr> my_units;
    map<int, Unit::Ptr> enemy_units;
    int round_id;
    bool isEat;
    double total_score;
    vector<vector<double>> power_score;
    vector<vector<double>> background_score;
    vector<vector<double>> background_score_limit;
    vector<vector<int>> visit_time;

};

#endif //AI_YANG_GAME_H
