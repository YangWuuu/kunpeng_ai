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
        auto unit = ri_p->my_units[mu];
        vector<Direction> choice_d;
        for (int i = 0; i < 5; i++){
            if (unit->loc->next[Direction(i)] != unit->loc) {
                choice_d.push_back(Direction(i));
            }
        }
        unit->direction = Direction::NONE;
        if (!choice_d.empty()) {
            unit->direction = choice_d[uniform_int_distribution<int>(0, choice_d.size() - 1)(e)];
        }
//        ri_p->my_units[mu]->direction = (Direction) (uniform_int_distribution<int>(0, 4)(e));
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
        auto move_step = [&](Direction direction) -> int {
            int min_distance = 0xffff;
            for (auto &eu : ri_p->enemy_units) {
                int d = Point::distance(eu.second->loc, unit->loc->next[direction]);
                if (d < min_distance) {
                    min_distance = d;
                }
            }
            return min_distance;
        };
        vector<Direction> choice_d;
        for (int i = 0; i < 5; i++){
            if (move_step(Direction(i)) > 1) {
                choice_d.push_back(Direction(i));
            }
        }
        Direction best_direction = Direction::NONE;
        int min_center_distance = 0xffff;
        Point::Ptr &center_point = leg_p->maps[leg_p->height / 2][leg_p->width / 2];
        for (Direction direction : choice_d) {
            int d = Point::distance(center_point, unit->loc->next[direction]);
            if (d < min_center_distance) {
                min_center_distance = d;
                best_direction = direction;
            }
        }
        unit->direction = best_direction;
        return BT::NodeStatus::SUCCESS;
    }
};


#endif //AI_YANG_CUSTOMACTION_H
