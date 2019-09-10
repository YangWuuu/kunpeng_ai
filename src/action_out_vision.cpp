#include "action_out_vision.h"

#include "player.h"

BT::NodeStatus OutVision::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->out_vision) {
        return BT::NodeStatus::SUCCESS;
    }

    vector<double> direction_score(info->task_score->score_num, 0.0);
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
        double score = 0.0;
        for (auto &eu : info->game->enemy_units_map) {
            int pos = eu.second;
            vector<double> &danger = info->game->vec_danger[pos];
            for (int loc : next_loc) {
                for (int index : info->leg_info->vision_grids[loc]) {
                    score -= danger[index];
                }
            }
        }
        direction_score[idx] = score;
    }

    info->task_score->set_task_score(TASK_NAME::TaskOutVision, direction_score);
    return BT::NodeStatus::SUCCESS;
}