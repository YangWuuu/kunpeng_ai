#ifndef AI_YANG_ACTION_REMOVE_INVALID_H
#define AI_YANG_ACTION_REMOVE_INVALID_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class RemoveInvalid : public BT::SyncActionNode {
public:
    explicit RemoveInvalid(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_REMOVE_INVALID_H
