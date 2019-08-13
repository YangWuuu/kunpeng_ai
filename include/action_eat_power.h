#ifndef AI_YANG_ACTION_EAT_POWER_H
#define AI_YANG_ACTION_EAT_POWER_H

#include <string>
#include <map>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "mcts.h"

using namespace std;

class EatPowerState {
public:
    bool is_terminal() const;

    int agent_id() const;

    void apply_action(const DIRECTION &action);

    void get_actions(vector<DIRECTION> &actions) const;

    bool get_random_action(DIRECTION &action) const;

    vector<double> evaluate() const;

    shared_ptr<LegStartInfo> leg_info;
    int agent_num{};
    vector<int> agent_loc;
    vector<double> agent_reward;
    map<int, double> loc_power;

    int round_id{0};  //from 0 every time
    bool _is_terminal{false};
    int _agent_id{0};
};

class EatPower : public BT::SyncActionNode {
public:
    explicit EatPower(const string &name, const BT::NodeConfiguration &config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_EAT_POWER_H
