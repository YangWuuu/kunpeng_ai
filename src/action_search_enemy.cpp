#include "action_search_enemy.h"

#include "player.h"

BT::NodeStatus SearchEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->search_enemy) {
        return BT::NodeStatus::SUCCESS;
    }

    vector<double> direction_score(info->task_score->score_num, 0.0);
    for (int idx = 0; idx < info->enemy_task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->enemy_task_score->map_direction[idx];

    }

    info->task_score->set_task_score(TASK_NAME::TaskSearchEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
