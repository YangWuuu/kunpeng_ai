#include "action_eat_power.h"

#include "player.h"
#include "log.h"

bool EatPowerState::is_terminal() const {
    return _is_terminal;
}

int EatPowerState::agent_id() const {
    return _agent_id;
}

void EatPowerState::apply_action(const DIRECTION &action) {
    Point::Ptr loc = leg_info->path.to_point(agent_loc[_agent_id]);
    int next_loc_int = leg_info->path.to_index(loc->next[action]);
    agent_loc[_agent_id] = next_loc_int;
    if (loc_power.count(next_loc_int) > 0) {
        agent_reward[_agent_id] += loc_power[next_loc_int];
        loc_power.erase(loc_power.find(next_loc_int));
    }
    _agent_id++;
    if (_agent_id == agent_num) {
        _agent_id = 0;
        round_id++;
        if (loc_power.empty()) {
            _is_terminal = true;
        }
    }
}

void EatPowerState::get_actions(vector<DIRECTION> &actions) const {
    actions.clear();
    auto loc = leg_info->path.to_point(agent_loc[_agent_id]);
    actions.emplace_back(DIRECTION::NONE);
    for (int i = 0; i < 4; i++) {
        if (loc->next[DIRECTION(i)] != loc) {
            actions.emplace_back(DIRECTION(i));
        }
    }
}

bool EatPowerState::get_random_action(DIRECTION &action) const {
    vector<DIRECTION> actions;
    get_actions(actions);
    if (actions.empty()) {
        return false;
    }
    action = actions[uniform_int_distribution<int>(0, actions.size() - 1)(e)];
    return true;
}

vector<double> EatPowerState::evaluate() const {
    vector<double> ret;
    for (double reward : agent_reward) {
        ret.emplace_back(reward / (round_id + 1));
    }
    return ret;
}

BT::NodeStatus EatPower::tick() {
    auto info = config().blackboard->get<Player *>("info");

    EatPowerState state;
    state.leg_info = info->leg_info;
    int my_units_count = 0;
    vector<int> my_units_id;
    for (auto &mu : info->round_info->my_units) {
        my_units_id.emplace_back(mu.first);
        state.agent_loc.emplace_back(info->leg_info->path.to_index(mu.second->loc));
        state.agent_reward.emplace_back(0.0);
        my_units_count++;
    }
    state.agent_num = my_units_count;
    for (auto &power : info->round_info->powers) {
        state.loc_power[info->leg_info->path.to_index(power.loc)] = (double) power.point;
    }
    mcts::UCT<EatPowerState, DIRECTION> uct;
    auto root_tree = uct.run(state);
    log_info("iter: %d", uct.get_iterations());
    auto node = uct.get_most_visited_child(root_tree);
    map<int, DIRECTION> units_direction;
    while (true) {
        units_direction[my_units_id[node->agent_id]] = node->get_action();
        if (node->get_state().round_id > 0) {
            break;
        }
        node = uct.get_most_visited_child(node);
    }
    vector<pair<map<int, DIRECTION>, double>> direction_score = {make_pair(units_direction, node->get_num_visits())};
    info->task_score->set_task_good_score(TASK_NAME::TaskEatPower, direction_score);
    return BT::NodeStatus::SUCCESS;
}
