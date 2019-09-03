#include "assign_task.h"

#include "player.h"

BT::NodeStatus AssignTask::tick() {
    auto info = config().blackboard->get<Player *>("info");
    if (info->record->enemy_all_remain_life <= 1 && info->round_info->round_id < 280) {
        info->record->eat_enemy = false;
    }
    return BT::NodeStatus::SUCCESS;
}
