#include "assign_task.h"

#include "player.h"

BT::NodeStatus AssignTask::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
