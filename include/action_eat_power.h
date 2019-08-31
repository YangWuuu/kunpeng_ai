#ifndef AI_YANG_ACTION_EAT_POWER_H
#define AI_YANG_ACTION_EAT_POWER_H

#include <string>
#include <map>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class EatPower : public BT::SyncActionNode {
public:
    explicit EatPower(const string &name, const BT::NodeConfiguration &config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_EAT_POWER_H
