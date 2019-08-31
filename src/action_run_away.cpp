#include "action_run_away.h"

#include "player.h"

BT::NodeStatus RunAway::tick() {
    auto info = config().blackboard->get<Player *>("info");
    if (info->game->is_eat) {
        return BT::NodeStatus::SUCCESS;
    }

    map<int, double> map_danger;
    for (int loc = 0; loc < info->leg_info->path.node_num; loc++) {
        for (const auto &enemy : info->round_info->enemy_units) {
            if (map_danger.count(loc) == 0) {
                map_danger[loc] = 0.0;
            }
            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 0 ||
                info->leg_info->path.get_cost(enemy.second->loc, loc) == 0) {
                map_danger[loc] -= 20;
            }
            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 1 ||
                info->leg_info->path.get_cost(enemy.second->loc, loc) == 1) {
                map_danger[loc] -= 10;
            }
            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 2 ||
                info->leg_info->path.get_cost(enemy.second->loc, loc) == 2) {
                map_danger[loc] -= 0.001;
            }
            if (info->leg_info->path.get_cost(loc, enemy.second->loc) == 3 ||
                info->leg_info->path.get_cost(enemy.second->loc, loc) == 3) {
                map_danger[loc] -= 0.0001;
            }
        }
    }

    vector<double> direction_score(info->task_score->score_num, 0.0);
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        for (auto &map_dir : map_direction) {
            auto next_point = info->round_info->my_units[map_dir.first]->loc->next[map_dir.second];
            direction_score[idx] += map_danger[next_point->index];
        }
    }

    info->task_score->set_task_good_score(TASK_NAME::TaskRunAway, direction_score);
    return BT::NodeStatus::SUCCESS;
}
