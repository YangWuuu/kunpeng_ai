#ifndef AI_YANG_ACTION_EAT_ENEMY_H
#define AI_YANG_ACTION_EAT_ENEMY_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class EatEnemy : public BT::SyncActionNode {
public:
    explicit EatEnemy(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_EAT_ENEMY_H
