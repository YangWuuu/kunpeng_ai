#include "game.h"

#include <algorithm>

void Game::update_round_info(shared_ptr<RoundInfo> &round_info) {
    round_id = round_info->round_id;
    is_eat = round_info->mode == leg_info->my_team.force;
    my_units.clear();
    //TODO enemy predict
    enemy_units.clear();
    for (auto &u : round_info->my_units) {
        auto &unit = *u.second;
        my_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
    }
    for (auto &u : round_info->enemy_units) {
        auto &unit = *u.second;
        enemy_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
    }
    for (int i = 0; i < leg_info->width; i++) {
        for (int j = 0; j < leg_info->height; j++) {
            if (round_id - visit_time[i][j] > 15) {
                env_score[i][j] = min(env_score_limit[i][j] / 40 * (round_id - 15 - visit_time[i][j]), env_score_limit[i][j]);
            }
        }
    }
    for (auto &unit : my_units) {
        for (const auto &xy : get_vision_grids(unit.second->loc, leg_info)) {
            power_score[xy.first][xy.second] = 0.0;
            env_score[xy.first][xy.second] = 0.0;
            visit_time[xy.first][xy.second] = round_id;
        }
    }
    for (auto &power : round_info->powers) {
        env_score_limit[power.loc->x][power.loc->y] = (double)power.point / (leg_info->vision * leg_info->vision);
        power_score[power.loc->x][power.loc->y] = power.point;
    }
}
