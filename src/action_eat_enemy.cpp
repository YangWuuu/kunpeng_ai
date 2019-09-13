#include <util.h>
#include "action_eat_enemy.h"

#include "player.h"

vector<vector<int>> choose_three(const vector<int> &now_loc, const vector<int> &next_loc) {
    if (next_loc.size() <= 3) {
        return {next_loc};
    }
    vector<vector<int>> ret;
    vector<vector<int>> choose_index = {{0, 1, 2}, {0, 1, 3}, {0, 2, 3}, {1, 2, 3}};
    for (auto &index : choose_index) {
        bool is_drop = true;
        for (int i : index) {
            if (now_loc[i] != next_loc[i]) {
                is_drop = false;
                break;
            }
        }
        if (!is_drop) {
            ret.emplace_back(vector<int>({next_loc[index[0]], next_loc[index[1]], next_loc[index[2]]}));
        }
    }
    return ret;
}

BT::NodeStatus EatEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->eat_enemy) {
        return BT::NodeStatus::SUCCESS;
    }

    vector<double> direction_score(info->task_score->score_num, 0.0);
    vector<int> now_loc;
    for (auto &mu : info->round_info->my_units) {
        now_loc.emplace_back(mu.second->loc->index);
    }
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        vector<int> next_loc;
        bool continue_flag = false;
        for (auto &mu : info->round_info->my_units) {
            auto next_point = mu.second->loc->next_point(map_direction[mu.first], info->game->map_first_cloud[mu.first]);
            if (map_direction[mu.first] != DIRECTION::NONE && next_point == mu.second->loc) {
                continue_flag = true;
                break;
            }
            next_loc.emplace_back(next_point->index);
        }
        if (continue_flag) {
            continue;
        }
        sort(next_loc.begin(), next_loc.end());
        for (vector<int> &choose_next_loc : choose_three(now_loc, next_loc)) {
            for (auto &eu : info->round_info->enemy_units) {
                double min_score = numeric_limits<double>::max();
                for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
                    auto next_point = eu.second->loc->next_point(d, info->game->map_first_cloud[eu.first]);
                    if (d != DIRECTION::NONE && next_point == eu.second->loc) {
                        continue;
                    }
                    int enemy_loc = next_point->index;
                    if (info->game->map_enemy_predict[eu.first]) {
                        enemy_loc = info->game->map_enemy_loc[eu.first];
//                        log_info("eu_id: %d enemy_loc: %d predict: %d", eu.first, next_point->index, enemy_loc);
                    }
                    double score;
                    if (find(now_loc.begin(), now_loc.end(), enemy_loc) != now_loc.end() ||
                        find(choose_next_loc.begin(), choose_next_loc.end(), enemy_loc) != choose_next_loc.end()) {
                        score = 10000;  // must be dead
                    }
                    else {
                        int remain_loc_num = info->leg_info->path.get_intersection_size(choose_next_loc, enemy_loc);
                        double total_dis = 0.0;
                        for (auto& nl : choose_next_loc) {
                            total_dis += info->leg_info->path.get_cost(nl, enemy_loc);
                        }
                        score = 40000.0 / (5.0 + remain_loc_num) + 400.0 / (1.0 + total_dis);
                    }
                    if (score < min_score) {
                        min_score = score;
                    }
                }
                direction_score[idx] = max({min_score, direction_score[idx]});
//            direction_score[idx] += min_score;
            }
        }
    }

    info->task_score->set_task_score(TASK_NAME::TaskEatEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
