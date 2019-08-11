#ifndef AI_YANG_ACTION_EXPLORE_MAP_H
#define AI_YANG_ACTION_EXPLORE_MAP_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class ExploreMap : public BT::SyncActionNode {
public:
    explicit ExploreMap(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_EXPLORE_MAP_H
