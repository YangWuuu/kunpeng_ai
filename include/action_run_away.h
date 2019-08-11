#ifndef AI_YANG_ACTION_RUN_AWAY_H
#define AI_YANG_ACTION_RUN_AWAY_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class RunAway : public BT::SyncActionNode {
public:
    explicit RunAway(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_RUN_AWAY_H
