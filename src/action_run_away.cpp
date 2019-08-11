#include "action_run_away.h"

#include "player.h"

BT::NodeStatus RunAway::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
