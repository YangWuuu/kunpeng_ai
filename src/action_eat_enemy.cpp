#include "action_eat_enemy.h"

#include "player.h"
#include "log.h"

bool EatEnemyState::is_terminal() const {
    return _is_terminal;
}

int EatEnemyState::agent_id() const {
    return _agent_id;
}

void EatEnemyState::apply_action(const DIRECTION &action) {
    Point::Ptr loc = leg_info->path.to_point(agent_loc[_agent_id]);
    int next_loc_int = leg_info->path.to_index(loc->next[action]);
    agent_loc[_agent_id] = next_loc_int;
    _agent_id++;
    if (_agent_id == agent_num) {
        _agent_id = 0;
        round_id++;
        for (int enemy_agent_id : enemy_agents_id) {
            for (int my_agent_id : my_agents_id) {
                if (leg_info->path.to_point(agent_loc[enemy_agent_id]) == leg_info->path.to_point(agent_loc[my_agent_id])) {
                    if (!equal_double(agent_reward[enemy_agent_id], 0.0)) {
                        agent_reward[enemy_agent_id] = 0.0;
                        agent_reward[my_agent_id] += agent_reward[enemy_agent_id];
                        _is_terminal = true;
                    }
                }
            }
        }
    }
}

void EatEnemyState::get_actions(vector<DIRECTION> &actions) {
    actions.clear();
    auto loc = leg_info->path.to_point(agent_loc[_agent_id]);
    if (round_id == 0 && my_agents_id.count(_agent_id) > 0) {
        actions.emplace_back(DIRECTION::NONE);
        for (int i = 0; i < 4; i++) {
            if (loc->next[DIRECTION(i)] != loc) {
                actions.emplace_back(DIRECTION(i));
            }
        }
    } else if (my_agents_id.count(_agent_id) > 0) {
        actions = my_move(agent_loc[_agent_id]);
    } else {
        actions = enemy_move(agent_loc[_agent_id]);
    }
}

bool EatEnemyState::get_random_action(DIRECTION &action) {
    vector<DIRECTION> actions;
    get_actions(actions);
    if (actions.empty()) {
        return false;
    }
    action = actions[uniform_int_distribution<int>(0, actions.size() - 1)(e)];
    return true;
}

vector<double> EatEnemyState::evaluate() const {
    vector<double> ret;
    for (double reward : agent_reward) {
        ret.emplace_back(reward / (round_id + 1));
    }
    return ret;
}

vector<DIRECTION> EatEnemyState::my_move(int loc_int) {
    auto loc = leg_info->path.to_point(loc_int);
    vector<DIRECTION> ret;
    for (int i = 0; i < 5; i++) {
        bool flag = false;
        for (int enemy_agent_id : enemy_agents_id) {
            auto enemy_loc = leg_info->path.to_point(agent_loc[enemy_agent_id]);
            if (leg_info->path.get_cost(loc->next[DIRECTION(i)], enemy_loc) <= 1 || leg_info->path.get_cost(enemy_loc, loc->next[DIRECTION(i)]) <= 1 || leg_info->path.get_cost(loc->next[DIRECTION(i)], enemy_loc) < leg_info->path.get_cost(loc, enemy_loc)) {
                flag = true;
                break;
            }
        }
        if (flag && (DIRECTION(i) == DIRECTION::NONE || (DIRECTION(i) != DIRECTION::NONE && loc->next[DIRECTION(i)] != loc))) {
            ret.emplace_back(DIRECTION(i));
        }
    }
    if (ret.empty()) {
        ret.emplace_back(DIRECTION::NONE);
    }
    return ret;
}

vector<DIRECTION> EatEnemyState::enemy_move(int loc_int) {
    auto loc = leg_info->path.to_point(loc_int);
    vector<DIRECTION> ret;
    for (int i = 0; i < 5; i++) {
        bool flag = true;
        for (int my_agent_id : my_agents_id) {
            auto my_loc = leg_info->path.to_point(agent_loc[my_agent_id]);
            if (leg_info->path.get_cost(loc->next[DIRECTION(i)], my_loc) == 0) {
                flag = false;
                break;
            }
        }
        if (flag && (DIRECTION(i) == DIRECTION::NONE || (DIRECTION(i) != DIRECTION::NONE && loc->next[DIRECTION(i)] != loc))) {
            ret.emplace_back(DIRECTION(i));
        }
    }
    if (ret.empty()) {
        ret.emplace_back(DIRECTION::NONE);
    }
    return ret;
}

BT::NodeStatus EatEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");
//
//    EatEnemyState state;
//    state.leg_info = info->leg_info;
//    int units_count = 0;
//    vector<int> my_units_id;
//    for (auto &mu : info->round_info->my_units) {
//        my_units_id.emplace_back(mu.first);
//        state.agent_loc.emplace_back(info->leg_info->path.to_index(mu.second->loc));
//        state.agent_reward.emplace_back(0.0);
//        state.my_agents_id.insert(units_count);
//        units_count++;
//    }
//    for (auto &eu : info->round_info->enemy_units) {
//        state.agent_loc.emplace_back(info->leg_info->path.to_index(eu.second->loc));
//        state.agent_reward.emplace_back(10.0 + eu.second->score);
//        state.enemy_agents_id.insert(units_count);
//        units_count++;
//    }
//    state.agent_num = units_count;
//
//    mcts::UCT<EatEnemyState, DIRECTION> uct;
//    uct.max_iterations = 30000;
//    uct.max_millis = 40;
//    uct.simulation_depth = 10 * units_count;
//    auto root_tree = uct.run(state);
//    log_info("iterations: %d/%d simulation_depth: %d run_millis: %.1f/%dms", uct.get_iterations(), uct.max_iterations,
//             uct.simulation_depth, uct.run_millis, uct.max_millis);
//
//    vector<pair<map<int, DIRECTION>, double>> direction_score;
//    function<void(decltype(root_tree) &, pair<map<int, DIRECTION>, double>)> save_all_path;
//    save_all_path = [&](decltype(root_tree) &node, pair<map<int, DIRECTION>, double> _score) {
//        if (node->get_parent()) {
//            int node_id = node->get_parent()->get_state().agent_id();
//            if (find(my_units_id.begin(), my_units_id.end(), node_id) != my_units_id.end()) {
//                _score.first[my_units_id[node_id]] = node->get_action();
//                _score.second = _score.second + node->get_value() / node->get_num_visits();
//            }
//            if (node->get_state().round_id > 0) {
//                direction_score.emplace_back(_score);
//                return;
//            }
//        }
//        int num_children = node->get_num_children();
//        for (int i = 0; i < num_children; i++) {
//            auto child = node->get_child(i);
//            save_all_path(child, _score);
//        }
//    };
//    pair<map<int, DIRECTION>, double> score;
//    save_all_path(root_tree, score);
//    info->task_score->set_task_good_score(TASK_NAME::TaskEatEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
