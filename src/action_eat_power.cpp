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

vector<double> EatPowerState::evaluate() {
    vector<double> ret;
    vector<int> power_loc;
    for (auto loc : loc_power) {
        power_loc.emplace_back(loc.first);
    }
    int i = 0;
    for (int s_agent_loc : agent_loc) {
        if (power_loc.empty()) {
            break;
        }
        int min_cost_loc = -1;
        int min_cost = numeric_limits<int>::max();
        for (auto s_power_loc: power_loc) {
            int cost = leg_info->path.get_cost(s_agent_loc, s_power_loc);
            if (cost < min_cost) {
                min_cost = cost;
                min_cost_loc = s_power_loc;
            }
        }
        power_loc.erase(find(power_loc.begin(), power_loc.end(), min_cost_loc));
        if (loc_power.count(min_cost_loc) > 0) {
            agent_loc_reward[i] = 1.0 * loc_power[min_cost_loc] / (double) (min_cost + 1);
        }
        i++;
    }
    i = 0;
    for (double reward : agent_reward) {
        ret.emplace_back(reward / (round_id + 1) + agent_loc_reward[i]);
        i++;
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
        state.agent_loc_reward.emplace_back(0.0);
        my_units_count++;
    }
    state.agent_num = my_units_count;
    for (auto &power : info->round_info->powers) {
        state.loc_power[info->leg_info->path.to_index(power.loc)] = (double) power.point;
    }

    mcts::UCT<EatPowerState, DIRECTION> uct;
    uct.max_iterations = 30000;
    uct.max_millis = 40;
    uct.simulation_depth = 2 * my_units_count;
    auto root_tree = uct.run(state);
    log_info("iterations: %d/%d simulation_depth: %d run_millis: %.1f/%dms", uct.get_iterations(), uct.max_iterations,
             uct.simulation_depth, uct.run_millis, uct.max_millis);

    vector<pair<map<int, DIRECTION>, double>> direction_score;
    function<void(decltype(root_tree) &, pair<map<int, DIRECTION>, double>)> save_all_path;
    save_all_path = [&](decltype(root_tree) &node, pair<map<int, DIRECTION>, double> _score) {
        if (node->get_parent()) {
            int node_id = node->get_parent()->get_state().agent_id();
            _score.first[my_units_id[node_id]] = node->get_action();
            _score.second = _score.second + node->get_value() / node->get_num_visits();
            if (node->get_state().round_id > 0) {
                direction_score.emplace_back(_score);
                return;
            }
        }
        int num_children = node->get_num_children();
        for (int i = 0; i < num_children; i++) {
            auto child = node->get_child(i);
            save_all_path(child, _score);
        }
    };
    pair<map<int, DIRECTION>, double> score;
    save_all_path(root_tree, score);
    info->task_score->set_task_good_score(TASK_NAME::TaskEatPower, direction_score);
    return BT::NodeStatus::SUCCESS;
}
