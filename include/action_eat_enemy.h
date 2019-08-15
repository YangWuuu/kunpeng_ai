#ifndef AI_YANG_ACTION_EAT_ENEMY_H
#define AI_YANG_ACTION_EAT_ENEMY_H

#include <string>

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "mcts.h"

using namespace std;

class EatEnemyState {
public:
    bool is_terminal() const;

    int agent_id() const;

    void apply_action(const DIRECTION &action);

    void get_actions(vector<DIRECTION> &actions);

    bool get_random_action(DIRECTION &action);

    vector<double> evaluate() const;

    vector<DIRECTION> my_move(int loc_int);
    vector<DIRECTION> enemy_move(int loc_int);

    shared_ptr<LegStartInfo> leg_info;
    int agent_num{};
    vector<int> agent_loc;
    set<int> my_agents_id;
    set<int> enemy_agents_id;
    vector<double> agent_reward;

    int round_id{0};  //from 0 every time
    bool _is_terminal{false};
    int _agent_id{0};
};

class EatEnemy : public BT::SyncActionNode {
public:
    explicit EatEnemy(const string &name, const BT::NodeConfiguration& config) :
            SyncActionNode(name, config) {}

    BT::NodeStatus tick() override;
};

#endif //AI_YANG_ACTION_EAT_ENEMY_H
