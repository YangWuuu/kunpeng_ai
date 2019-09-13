#include "action_out_vision.h"

#include "player.h"

BT::NodeStatus OutVision::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->out_vision) {
        return BT::NodeStatus::SUCCESS;
    }
    vector<int> now_loc;
    for (auto &mu : info->round_info->my_units) {
        now_loc.emplace_back(mu.second->loc->index);
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
        for (int e_loc = 0; e_loc < info->leg_info->path.node_num; e_loc++) {
            double danger = info->game->all_danger[e_loc];
            if (danger < 1e-2) {
                continue;
            }
            for (int i_loc : next_loc) {
                double i_dis = info->leg_info->path.get_cost(e_loc, i_loc);
                score -= (40 - i_dis) * danger;
            }
        }
        direction_score[idx] = score;
    }

    info->task_score->set_task_score(TASK_NAME::TaskOutVision, direction_score);
    return BT::NodeStatus::SUCCESS;
}
