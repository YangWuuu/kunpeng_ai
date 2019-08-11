#include "action_eat_enemy.h"

#include "player.h"

BT::NodeStatus EatEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
