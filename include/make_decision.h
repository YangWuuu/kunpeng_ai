#ifndef AI_YANG_MAKE_DECISION_H
#define AI_YANG_MAKE_DECISION_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class MakeDecision : public BT::SyncActionNode {
public:
    explicit MakeDecision(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};


#endif //AI_YANG_MAKE_DECISION_H
