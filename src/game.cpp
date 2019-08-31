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
    for (int n = 0; n < leg_info->path.node_num; n++) {
        if (round_id - visit_time[n] > 15) {
            env_score[n] = min(env_score_limit[n] / 25 * (round_id - 15 - visit_time[n]), env_score_limit[n]);
        }
    }
    for (auto &unit : my_units) {
        for (int n : leg_info->vision_grids[unit.second->loc->index]) {
            power_score[n] = 0.0;
            env_score[n] = 0.0;
            visit_time[n] = round_id;
        }
    }
    for (auto &power : round_info->powers) {
        env_score_limit[power.loc->index] = (double)power.point / (leg_info->vision * leg_info->vision);
        power_score[power.loc->index] = power.point;
    }
}
