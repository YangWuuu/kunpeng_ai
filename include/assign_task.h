#ifndef AI_YANG_ASSIGN_TASK_H
#define AI_YANG_ASSIGN_TASK_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class AssignTask : public BT::SyncActionNode {
public:
    explicit AssignTask(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ASSIGN_TASK_H
