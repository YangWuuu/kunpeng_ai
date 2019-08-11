#include "action_eat_power.h"

#include "player.h"

BT::NodeStatus EatPower::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
