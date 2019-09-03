#include "predict_enemy.h"

#include "player.h"

BT::NodeStatus PredictEnemyNowLoc::tick() {
    auto info = config().blackboard->get<Player *>("info");

    return BT::NodeStatus::SUCCESS;
}
