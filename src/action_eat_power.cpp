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

    map<int, double> map_power;
    for (const auto &power : info->round_info->powers) {
        map_power[info->leg_info->path.to_index(power.loc)] = power.point;
    }

    map<int, pair<int, int>> mu_time;
    for (const auto &mu : info->round_info->my_units) {
        mu_time[mu.first] = make_pair(0, info->leg_info->path.to_index(mu.second->loc));
    }
    map<int, int> mu_first_power;
    for (const auto &power : map_power) {
        int mu_id = 0;
        int closest_loc = 0;
        double shortest_dis = numeric_limits<double>::max();
        for (const auto &mu : info->round_info->my_units) {
            double dis = info->leg_info->path.get_cost(mu_time[mu.first].second, power.first) + mu_time[mu.first].first;
            if (dis < shortest_dis) {
                shortest_dis = dis;
                mu_id = mu.first;
                closest_loc = mu_time[mu.first].second;
            }
        }
        if (mu_first_power.count(mu_id) == 0) {
            mu_first_power[mu_id] = power.first;
        }
        mu_time[mu_id].first += shortest_dis;
        mu_time[mu_id].second = closest_loc;
    }
    struct dir_score {
        dir_score(int _mu_id, DIRECTION _dir, double _score) :
                mu_id(_mu_id), dir(_dir), score(_score) {}

        int mu_id;
        DIRECTION dir;
        double score;
    };
    vector<dir_score> single_direction_score;
    for (const auto &mu : mu_first_power) {
        for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT}) {
            double cost = info->leg_info->path.get_cost(info->round_info->my_units[mu.first]->loc->next[d], mu.second);
            double score = map_power[mu.second] / (cost + 1);
            single_direction_score.emplace_back(dir_score(mu.first, d, score));
        }
    }

    vector<pair<map<int, DIRECTION>, double>> direction_score;
    for (int id = 0; id < info->task_score->score_num; id++) {
        direction_score.emplace_back(make_pair(info->task_score->get_map_direction(id), 0.0));
    }
    for (const auto &sds : single_direction_score) {
        for (auto &ds : direction_score) {
            if (ds.first[sds.mu_id] == sds.dir) {
                ds.second += sds.score;
            }
        }
    }

    info->task_score->set_task_good_score(TASK_NAME::TaskEatPower, direction_score);
    return BT::NodeStatus::SUCCESS;
}
