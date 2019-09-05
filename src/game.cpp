#include "game.h"

#include <algorithm>

void Game::update_round_info(const shared_ptr<RoundInfo> &round_info) {
    vec_round_info.emplace_back(round_info);
    update_score();
    update_remain_life();
    update_danger();

    eat_enemy = false;
    run_away = false;
    search_enemy = false;
    avoid_enemy = false;
}

void Game::update_score() {
    auto &round_info = *(vec_round_info.end() - 1);
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

void Game::update_remain_life() {
    auto &round_info = *(vec_round_info.end() - 1);
    my_all_remain_life = (int)round_info->my_units.size() + round_info->my_remain_life;
    if (vec_round_info.size() == 1) {
        enemy_all_remain_life = 4 + round_info->enemy_remain_life;
    } else {
        auto &prev_round_info = *(vec_round_info.end() - 2);
        if (is_eat) {
            int prev_round_unit_score = 0;
            int round_unit_score = 0;
            for (auto &mu : prev_round_info->my_units) {
                prev_round_unit_score += mu.second->score;
            }
            for (auto &mu : round_info->my_units) {
                round_unit_score += mu.second->score;
            }
            int sub_score = (round_info->my_point - prev_round_info->my_point) - (round_unit_score - prev_round_unit_score);
            if (sub_score > 0) {
                enemy_all_remain_life -= sub_score / 10;
                log_info("round_id: %d sub_score: %d enemy_all_remain_life: %d", round_info->round_id, sub_score, enemy_all_remain_life);
            }
            if (sub_score % 10 != 0) {
                log_error("sub_score: %d", sub_score);
            }
        }
    }
}

void Game::update_danger() {
    auto &round_info = *(vec_round_info.end() - 1);
    for (auto &eu : enemy_units_map) {
        int id = eu.first;
        int pos = eu.second;
        vector<double> &danger = vec_danger[pos];
        auto iter = round_info->enemy_units.find(id);
        if (iter != round_info->enemy_units.end()) {
            danger = vector<double>(leg_info->path.node_num, 0.0);
            danger[iter->second->loc->index] = leg_info->path.node_num;
        } else {
            vector<double> danger_tmp(leg_info->path.node_num, 0.0);
            vector<int> next_index;
            for (int i = 0; i < leg_info->path.node_num; i++) {
                Point::Ptr p = leg_info->path.to_point(i);
                if (p->tunnel != DIRECTION::NONE || p->wall) {
                    continue;
                }
                if (equal_double(danger[i], 1e-6)) {
                    continue;
                }
                next_index.clear();
                for (auto &np : p->next) {
                    if (np.second->tunnel != DIRECTION::NONE || np.second->wall) {
                        continue;
                    }
                    next_index.emplace_back(np.second->index);
                }
                for (int ni : next_index) {
                    danger_tmp[ni] += danger[i] / next_index.size();
                }
            }
            danger = danger_tmp;
        }
    }
    vector<bool> vision_grids_index(leg_info->path.node_num, false);
    for (auto &mu : round_info->my_units) {
        for (int index : leg_info->vision_grids[mu.second->loc->index]) {
            Point::Ptr p = leg_info->path.to_point(index);
            if (p->tunnel != DIRECTION::NONE || p->wall) {
                continue;
            }
            vision_grids_index[index] = true;
        }
    }
    for (auto &eu : enemy_units_map) {
        int id = eu.first;
        int pos = eu.second;
        vector<double> &danger = vec_danger[pos];
        auto iter = round_info->enemy_units.find(id);
        if (iter == round_info->enemy_units.end()) {
            double inside = 0.0;
            double outside = 0.0;
            int outside_count = 0;
            for (int i = 0; i < leg_info->path.node_num; i++) {
                Point::Ptr p = leg_info->path.to_point(i);
                if (p->tunnel != DIRECTION::NONE || p->wall) {
                    continue;
                }
                if (vision_grids_index[i]) {
                    inside += danger[i];
                    danger[i] = 0.0;
                } else {
                    outside += danger[i];
                    outside_count++;
                }
            }
            //may be dead
            for (int i = 0; i < leg_info->path.node_num; i++) {
                Point::Ptr p = leg_info->path.to_point(i);
                if (p->tunnel != DIRECTION::NONE || p->wall) {
                    continue;
                }
                if (!vision_grids_index[i]) {
                    danger[i] += inside / outside_count;
                }
            }
            log_error("id: %d inside: %f outside: %f", eu.first, inside, outside);
        }
    }
}
