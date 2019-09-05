#include "assign_task.h"

#include "player.h"

BT::NodeStatus AssignTask::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (info->game->round_id < 150 && info->game->is_eat) {
        if (info->game->enemy_all_remain_life > 1) {
            info->game->eat_enemy = true;
        } else {
//            info->game->avoid_enemy = true;
        }
    }
    if (info->game->round_id >= 150 && !info->game->is_eat) {
        info->game->run_away = true;
//        info->game->avoid_enemy = true;
    }
    if (info->game->round_id < 150 && !info->game->is_eat) {
        if (info->game->my_all_remain_life > 1) {
            info->game->run_away = true;
        }
    }
    if (info->game->round_id >= 150 && info->game->is_eat) {
        if (info->game->my_all_remain_life > 1 && (info->game->enemy_all_remain_life > 1 || info->round_info->round_id >= 280)) {
            info->game->eat_enemy = true;
        }
    }

    return BT::NodeStatus::SUCCESS;
}
