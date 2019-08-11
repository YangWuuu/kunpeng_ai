#include "customAction.h"

#include "player.h"

BT::NodeStatus CalculateShortestPath::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus PredictEnemyNowLoc::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
