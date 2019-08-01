#ifndef AI_YANG_CUSTOMCONDITION_H
#define AI_YANG_CUSTOMCONDITION_H


#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class IsEnemyAround : public BT::ConditionNode {
public:
    explicit IsEnemyAround(const string &name, const BT::NodeConfiguration& config) :
            ConditionNode(name, config) {}

    BT::NodeStatus tick() override {
        auto mu = config().blackboard->get<int>("mu");
        auto leg_p = config().blackboard->get<LegStartInfo *>("leg");
        auto ri_p = config().blackboard->get<RoundInfo *>("ri");
        auto unit = ri_p->my_units[mu];
        Point::Ptr nearest;
        int min_distance = 0xffff;
        for (auto &eu : ri_p->enemy_units) {
            int d = Point::distance(eu.second->loc, unit->loc);
            if (d < min_distance) {
                min_distance = d;
                nearest = eu.second->loc;
            }
        }
        if (min_distance <= 3) {
            return BT::NodeStatus::SUCCESS;
        } else {
            return BT::NodeStatus::FAILURE;
        }
    }
};

class IsMyMode : public BT::ConditionNode {
public:
    explicit IsMyMode(const string &name, const BT::NodeConfiguration& config) :
            ConditionNode(name, config) {}

    BT::NodeStatus tick() override {

        auto mu = config().blackboard->get<int>("mu");
        auto leg_p = config().blackboard->get<LegStartInfo *>("leg");
        auto ri_p = config().blackboard->get<RoundInfo *>("ri");
        if (ri_p->mode == leg_p->my_team.force) {
            return BT::NodeStatus::SUCCESS;
        } else {
            return BT::NodeStatus::FAILURE;
        }
    }
};

class IsPowerAround : public BT::ConditionNode {
public:
    explicit IsPowerAround(const string &name, const BT::NodeConfiguration& config) :
            ConditionNode(name, config) {}

    BT::NodeStatus tick() override {
        auto mu = config().blackboard->get<int>("mu");
        auto leg_p = config().blackboard->get<LegStartInfo *>("leg");
        auto ri_p = config().blackboard->get<RoundInfo *>("ri");
        auto unit = ri_p->my_units[mu];
        Point::Ptr nearest;
        int min_distance = 0xffff;
        for (auto &p : ri_p->powers) {
            int d = Point::distance(p.loc, unit->loc);
            if (d < min_distance) {
                min_distance = d;
                nearest = p.loc;
            }
        }
        if (min_distance <= leg_p->vision) {
            return BT::NodeStatus::SUCCESS;
        } else {
            return BT::NodeStatus::FAILURE;
        }
    }
};

#endif //AI_YANG_CUSTOMCONDITION_H
