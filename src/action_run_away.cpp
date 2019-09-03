#include "action_run_away.h"

#include "player.h"

BT::NodeStatus RunAway::tick() {
    auto info = config().blackboard->get<Player *>("info");
    if (info->game->is_eat) {
        return BT::NodeStatus::SUCCESS;
    }

//    map<int, double> map_danger;
//    for (int loc = 0; loc < info->leg_info->path.node_num; loc++) {
//        for (const auto &enemy : info->round_info->enemy_units) {
//            if (map_danger.count(loc) == 0) {
//                map_danger[loc] = 0.0;
//            }
//            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 0 ||
//                info->leg_info->path.get_cost(enemy.second->loc, loc) == 0) {
//                map_danger[loc] -= 20;
//            }
//            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 1 ||
//                info->leg_info->path.get_cost(enemy.second->loc, loc) == 1) {
//                map_danger[loc] -= 10;
//            }
//            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 2 ||
//                info->leg_info->path.get_cost(enemy.second->loc, loc) == 2) {
//                map_danger[loc] -= 0.001;
//            }
//            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 3 ||
//                info->leg_info->path.get_cost(enemy.second->loc, loc) == 3) {
//                map_danger[loc] -= 0.0001;
//            }
//        }
//    }
//
//    vector<double> direction_score(info->task_score->score_num, 0.0);
//    for (int idx = 0; idx < info->task_score->score_num; idx++) {
//        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
//        for (auto &map_dir : map_direction) {
//            auto next_point = info->round_info->my_units[map_dir.first]->loc->next[map_dir.second];
//            direction_score[idx] += map_danger[next_point->index];
//        }
//    }

    vector<int> reach_size(info->leg_info->path.node_num, info->leg_info->path.node_num);
    for (auto &eu : info->round_info->enemy_units) {
        reach_size[eu.second->loc->index] = 0;
    }
    set<int> next_loc_set;
    for (auto &mu : info->round_info->my_units) {
        for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT}) {
            int next_loc = mu.second->loc->next[d]->index;
            next_loc_set.insert(next_loc);
        }
    }

    for (int idx = 0; idx < info->enemy_task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->enemy_task_score->map_direction[idx];
        vector<int> next_loc;
        bool continue_flag = false;
        for (auto &eu : info->round_info->enemy_units) {
            auto &next_point = eu.second->loc->next[map_direction[eu.first]];
            if (map_direction[eu.first] != DIRECTION::NONE && next_point == eu.second->loc) {
                continue_flag = true;
                break;
            }
            next_loc.emplace_back(next_point->index);
        }
        if (continue_flag) {
            continue;
        }
        sort(next_loc.begin(), next_loc.end());
        for (auto nl : next_loc) {
            reach_size[nl] = 0;
            Point::Ptr p = info->leg_info->path.to_point(nl)->wormhole;
            if (p) {
                reach_size[p->index] = 0;
            }
        }
        for (int index : next_loc_set) {
            int remain_loc_num = info->leg_info->path.get_intersection_size(next_loc, index);
            reach_size[index] = min(reach_size[index], remain_loc_num);
        }
    }
    auto get_score = [](int remain_loc_num) -> double {
        if (remain_loc_num < 5) {
            return -10000.0 * (5 - remain_loc_num);
        } else if (remain_loc_num < 20) {
            return remain_loc_num - 5;
        } else if (remain_loc_num < 40) {
            return 15.0 + (remain_loc_num - 15) * 0.001;
        } else {
            return 15.0 + 25 * 0.001;
        }
    };
    vector<double> direction_score(info->task_score->score_num, 0.0);
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        for (auto &map_dir : map_direction) {
            auto next_point = info->round_info->my_units[map_dir.first]->loc->next[map_dir.second];
            direction_score[idx] += get_score(reach_size[next_point->index]);
        }
    }

    info->task_score->set_task_good_score(TASK_NAME::TaskRunAway, direction_score);
    return BT::NodeStatus::SUCCESS;
}
