#ifndef AI_YANG_CUSTOMACTION_H
#define AI_YANG_CUSTOMACTION_H


#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"

using namespace std;

class RandomWalk : public BT::SyncActionNode {
public:
    explicit RandomWalk(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override {
        auto mu = config().blackboard->get<int>("mu");
        auto leg_p = config().blackboard->get<LegStartInfo *>("leg");
        auto ri_p = config().blackboard->get<RoundInfo *>("ri");
        ri_p->my_units[mu]->direction = (Direction) (uniform_int_distribution<int>(0, 4)(e));
        return BT::NodeStatus::SUCCESS;
    }
};

class MoveToNearestPower : public BT::SyncActionNode {
public:
    explicit MoveToNearestPower(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

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
        int dx = nearest->x - unit->loc->x;
        int dy = nearest->y - unit->loc->y;
        if (dy > 0) {
            unit->direction = Direction::DOWN;
        }else if(dy < 0){
            unit->direction = Direction::UP;
        }
        if (dx > 0) {
            unit->direction = Direction::RIGHT;
        }else if(dx < 0){
            unit->direction = Direction::LEFT;
        }
        return BT::NodeStatus::SUCCESS;
    }
};

class RunAway : public BT::SyncActionNode {
public:
    explicit RunAway(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

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
        if (min_distance <= 2){
            int dx = nearest->x - unit->loc->x;
            int dy = nearest->y - unit->loc->y;
            if (dy > 0) {
                unit->direction = Direction::UP;
            }else if(dy < 0){
                unit->direction = Direction::DOWN;
            }
            if (dx > 0) {
                unit->direction = Direction::LEFT;
            }else if(dx < 0){
                unit->direction = Direction::RIGHT;
            }
        }
        return BT::NodeStatus::SUCCESS;
    }
};


#endif //AI_YANG_CUSTOMACTION_H
