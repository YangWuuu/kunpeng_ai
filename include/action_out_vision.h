#ifndef AI_YANG_ACTION_OUT_VISION_H
#define AI_YANG_ACTION_OUT_VISION_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class OutVision : public BT::SyncActionNode {
public:
    explicit OutVision(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_OUT_VISION_H
