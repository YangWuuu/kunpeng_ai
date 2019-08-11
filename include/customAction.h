#ifndef AI_YANG_CUSTOMACTION_H
#define AI_YANG_CUSTOMACTION_H


#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class CalculateShortestPath : public BT::SyncActionNode {
public:
    explicit CalculateShortestPath(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

class PredictEnemyNowLoc : public BT::SyncActionNode {
public:
    explicit PredictEnemyNowLoc(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_CUSTOMACTION_H
