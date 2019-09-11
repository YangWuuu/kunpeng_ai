#include "game.h"

#include <algorithm>

void Game::update_round_info(const shared_ptr<RoundInfo> &round_info) {
    vec_round_info.emplace_back(round_info);
    is_eat = round_info->mode == leg_info->my_team.force;
    update_score();
    update_remain_life();
    update_map_power();
    update_danger();
    update_first_cloud();
    update_dist();

    eat_enemy = false;
    run_away = false;
    search_enemy = false;
    avoid_enemy = false;
    out_vision = false;
}

void Game::update_score() {
    auto &round_info = *(vec_round_info.end() - 1);
    round_id = round_info->round_id;
    my_units.clear();
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
        env_score_limit[power.loc->index] = (double) power.point / (leg_info->vision * leg_info->vision);
        power_score[power.loc->index] = power.point;
    }
}

void Game::update_remain_life() {
    auto &round_info = *(vec_round_info.end() - 1);
    my_all_remain_life = (int) round_info->my_units.size() + round_info->my_remain_life;
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
            int sub_score =
                    (round_info->my_point - prev_round_info->my_point) - (round_unit_score - prev_round_unit_score);
            if (sub_score > 0) {
                enemy_all_remain_life -= sub_score / 10;
                see_alive_enemy.clear();
//                log_info("round_id: %d sub_score: %d enemy_all_remain_life: %d", round_info->round_id, sub_score, enemy_all_remain_life);
            }
            if (sub_score % 10 != 0) {
                log_error("sub_score: %d", sub_score);
            }
        }
    }
    log_info("round_id: %d my_all_remain_life: %d enemy_all_remain_life: %d", round_info->round_id, my_all_remain_life, enemy_all_remain_life);
    for (auto &eu : round_info->enemy_units) {
        see_alive_enemy.insert(eu.first);
    }
    if ((int) see_alive_enemy.size() == enemy_all_remain_life) {
        for (int enemy_id : leg_info->enemy_team.units) {
            auto iter = see_alive_enemy.find(enemy_id);
            if (iter == see_alive_enemy.end()) {
                dead_enemy.insert(enemy_id);
            }
        }
    }
    if (!dead_enemy.empty()) {
        string log_str = "round_id: " + to_string(round_info->round_id) + " ";
        log_str += "enemy_all_remain_life: " + to_string(enemy_all_remain_life) + " ";
        log_str += "dead_enemy: ";
        for (int dead_enemy_id : dead_enemy) {
            log_str += to_string(dead_enemy_id) + " ";
        }
        log_info(log_str.c_str());
    }
}

void Game::update_map_power() {
    auto &round_info = *(vec_round_info.end() - 1);
    for (auto &unit : my_units) {
        for (int n : leg_info->vision_grids[unit.second->loc->index]) {
            auto iter = map_power.find(n);
            if (iter != map_power.end()) {
                map_power.erase(iter);
            }
        }
    }
    for (auto &power : round_info->powers) {
        map_power[power.loc->index] = power.point;
    }
}

void Game::update_danger() {
    auto &round_info = *(vec_round_info.end() - 1);

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
        if (dead_enemy.find(id) != dead_enemy.end()) {
            continue;
        }
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

    for (auto &eu : enemy_units_map) {
        int id = eu.first;
        int pos = eu.second;
        vector<double> &danger = vec_danger[pos];
        if (dead_enemy.find(id) != dead_enemy.end()) {
            danger = vector<double>(leg_info->path.node_num, 0.0);
            continue;
        }
        auto iter = round_info->enemy_units.find(id);
        if (iter != round_info->enemy_units.end()) {
            danger = vector<double>(leg_info->path.node_num, 0.0);
            danger[iter->second->loc->index] = leg_info->path.node_num * 100.0;
        }
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

        //twice
        if (is_eat) {
            vector<double> danger_tmp2(leg_info->path.node_num, 0.0);
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
                    danger_tmp2[ni] += danger[i] / next_index.size();
                }
            }
            danger = danger_tmp2;
        }
    }

    all_enemy_in_vision = false;
    if (round_info->enemy_units.size() + dead_enemy.size() >= 4) {
        all_enemy_in_vision = true;
    }

    int node_num = leg_info->path.node_num;
    danger_in_vision.assign(node_num, false);
    danger_eat_in_vision.assign(node_num, false);
    for (auto &eu : round_info->enemy_units) {
        int index = eu.second->loc->index;
        Point::Ptr p = leg_info->path.to_point(index);
        for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
            danger_in_vision[p->next[d]->index] = true;
            for (DIRECTION dd : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
                danger_eat_in_vision[p->next[d]->next[dd]->index] = true;
            }
        }
    }
}

void Game::update_first_cloud() {
    auto &round_info = *(vec_round_info.end() - 1);
    for (int mu_id : leg_info->my_team.units) {
        map_first_cloud[mu_id] = true;
    }
    for (int eu_id : leg_info->enemy_team.units) {
        map_first_cloud[eu_id] = true;
    }
    if (vec_round_info.size() > 1) {
        auto &prev_round_info = *(vec_round_info.end() - 2);
        for (auto &mu : round_info->my_units) {
            auto iter = prev_round_info->my_units.find(mu.first);
            if (iter != prev_round_info->my_units.end()) {
                if (!mu.second->loc->cloud) {
                    map_first_cloud[mu.first] = false;
                } else {
                    if (mu.second->loc->index == iter->second->loc->index) {
                        map_first_cloud[mu.first] = false;
                    }
                }
            }
        }
        for (auto &eu : round_info->enemy_units) {
            auto iter = prev_round_info->enemy_units.find(eu.first);
            if (iter != prev_round_info->enemy_units.end()) {
                if (!eu.second->loc->cloud) {
                    map_first_cloud[eu.first] = false;
                } else {
                    if (eu.second->loc->index == iter->second->loc->index) {
                        map_first_cloud[eu.first] = false;
                    }
                }
            }
        }
    }
}

void Game::update_dist() {
    auto &round_info = *(vec_round_info.end() - 1);
    int node_num = leg_info->path.node_num;
    G = vector<vector<double>>(node_num, vector<double>(node_num, inf));
    all_danger.assign(node_num, 0.0);
    for (auto &eu : enemy_units_map) {
        int id = eu.first;
        int pos = eu.second;
        vector<double> &danger = vec_danger[pos];
        if (dead_enemy.find(id) != dead_enemy.end()) {
            continue;
        }
        for (int i = 0; i < node_num; i++) {
            all_danger[i] += danger[i];
        }
    }
    set<int> next_loc_set;
    for (auto &eu : round_info->enemy_units) {
        for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
            int next_loc = eu.second->loc->next[d]->index;
            next_loc_set.insert(next_loc);
        }
    }
    for (int i = 0; i < node_num; i++) {
        for (auto &dp : leg_info->path.to_point(i)->next) {
            int j = dp.second->index;
            if (i != j) {
                if (leg_info->path.to_point(i)->cloud) {
                    G[i][j] = 2;
                } else {
                    G[i][j] = 1;
                }
                if (((is_eat && leg_info->path.is_eat_danger_index[j]) || (!is_eat && leg_info->path.is_danger_index[j]))
                    && !all_enemy_in_vision) {
                    G[i][j] = 1e6;
                }
                Point::Ptr p = leg_info->path.to_point(j)->wormhole;
                if (p) {
                    if (((is_eat && leg_info->path.is_eat_danger_index[p->index]) || (!is_eat && leg_info->path.is_danger_index[p->index]))
                        && !all_enemy_in_vision) {
                        G[i][j] = 1e6;
                    }
                }
                if (next_loc_set.find(j) != next_loc_set.end()) {
                    G[i][j] = 1e6;
                }
            } else if (i == j) {
                G[i][j] = 0;
            }
        }
    }

    dist.assign(node_num, vector<double>(node_num, inf));
    is_cal.assign(node_num, false);
}

bool Game::relax(int u, int v, vector<double> &d) {
    double nlen = d[u] + G[u][v];
    if (nlen < d[v]) {
        d[v] = nlen;
        return true;
    }
    return false;
}

void Game::SPFA(int k) {
    int node_num = leg_info->path.node_num;
    if (is_cal[k]) {
        return;
    }
    is_cal[k] = true;
    Point::Ptr point = leg_info->path.to_point(k);
    if (point->wall || point->tunnel != DIRECTION::NONE || point->wormhole) {
        return;
    }
    queue<int> Q;
    vector<bool> is_in_Q(node_num, false);

    Q.push(k);
    is_in_Q[k] = true;
    dist[k][k] = 0;
    while (!Q.empty()) {
        int u = Q.front();
        Q.pop();
        is_in_Q[u] = false;
        for (int v = 0; v < node_num; ++v) {
            if (relax(u, v, dist[k]) && !is_in_Q[v]) {
                Q.push(v);
                is_in_Q[v] = true;
            }
        }
    }
}

double Game::get_cost(int start, int end) {
    SPFA(start);
    return dist[start][end];
}
