#include "action_remove_invalid.h"

#include "player.h"

BT::NodeStatus RemoveInvalid::tick() {
    auto info = config().blackboard->get<Player *>("info");

    vector<double> direction_score(info->task_score->score_num, 0.0);
    if (info->task_score->score_num > 0) {
        direction_score[info->task_score->score_num - 1] = -1e6;
    }
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        bool continue_flag = false;
        for (auto &mu : info->round_info->my_units) {
            auto &next_point = mu.second->loc->next[map_direction[mu.first]];
            if (map_direction[mu.first] != DIRECTION::NONE && next_point == mu.second->loc) {
                continue_flag = true;
                break;
            }
        }
        if (continue_flag) {
            direction_score[idx] = -1e6;
        }
    }

    info->task_score->set_task_score(TASK_NAME::TaskRemoveInvalid, direction_score);
    return BT::NodeStatus::SUCCESS;
}
