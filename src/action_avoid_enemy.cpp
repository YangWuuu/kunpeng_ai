#include "action_avoid_enemy.h"

#include "player.h"

BT::NodeStatus AvoidEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->avoid_enemy) {
        return BT::NodeStatus::SUCCESS;
    }

    vector<double> direction_score(info->task_score->score_num, 0.0);
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
        double score = 0.0;
        for (int loc : next_loc) {
            if ((!info->game->is_eat && info->game->danger_in_vision[loc]) || (info->game->is_eat && info->game->danger_eat_in_vision[loc])){
                score -= 1e3;
            }
        }
        direction_score[idx] = score;
    }

    info->task_score->set_task_score(TASK_NAME::TaskAvoidEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
