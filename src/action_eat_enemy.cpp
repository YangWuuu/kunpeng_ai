#include "action_eat_enemy.h"

#include "player.h"

BT::NodeStatus EatEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->is_eat) {
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
            auto &next_point = mu.second->loc->next[map_direction[mu.first]];
            if (map_direction[mu.first] != DIRECTION::NONE && next_point == mu.second->loc) {
                continue_flag = true;
                break;
            }
            next_loc.emplace_back(next_point->index);
        }
        if (continue_flag) {
            continue;
        }
        for (auto &eu : info->round_info->enemy_units) {
            double min_score = numeric_limits<double>::max();
            for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
                auto &next_point = eu.second->loc->next[d];
                if (d != DIRECTION::NONE && next_point == eu.second->loc) {
                    continue;
                }
                int enemy_loc = next_point->index;
                double score;
                if (find(now_loc.begin(), now_loc.end(), enemy_loc) != now_loc.end() ||
                    find(next_loc.begin(), next_loc.end(), enemy_loc) != next_loc.end()) {
                    score = 10000;  // must be dead
                }
                else {
                    int remain_loc_num = 0;
                    for (int tmp_loc = 0; tmp_loc < info->leg_info->path.node_num; tmp_loc++) {
                        int enemy_dis = info->leg_info->path.get_cost(enemy_loc, tmp_loc);
                        bool in_reach = false;
                        for (auto& nl : next_loc) {
                            int tmp_dis = info->leg_info->path.get_cost(nl, tmp_loc);
                            if (tmp_dis <= enemy_dis) {
                                in_reach = true;
                                break;
                            }
                        }
                        if (!in_reach) {
                            remain_loc_num++;
                        }
                    }
                    double total_dis = 0.0;
                    for (auto& nl : next_loc) {
                        total_dis += info->leg_info->path.get_cost(nl, enemy_loc);
                    }
                    score = 40000.0 / (5.0 + remain_loc_num) + 400.0 / (1.0 + total_dis);
                }
                if (score < min_score) {
                    min_score = score;
                }
            }
            direction_score[idx] = max({min_score, direction_score[idx]});
        }
    }

    info->task_score->set_task_good_score(TASK_NAME::TaskEatEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
