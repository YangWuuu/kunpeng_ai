#include "action_explore_map.h"

#include "player.h"

BT::NodeStatus ExploreMap::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
