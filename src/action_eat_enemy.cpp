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

    if (!info->game->is_eat) {
        return BT::NodeStatus::SUCCESS;
    }

    vector<pair<map<int, DIRECTION>, double>> direction_score;
    for (int id = 0; id < info->task_score->score_num; id++) {
        direction_score.emplace_back(make_pair(info->task_score->get_map_direction(id), 0.0));
    }
    vector<int> now_loc;
    for (auto &mu : info->round_info->my_units) {
        now_loc.emplace_back(info->leg_info->path.to_index(mu.second->loc));
    }
    for (auto &p : direction_score) {
        vector<int> next_loc;
        bool continue_flag = false;
        for (auto &mu : info->round_info->my_units) {
            auto &next_point = mu.second->loc->next[p.first[mu.first]];
            if (p.first[mu.first] != DIRECTION::NONE && next_point == mu.second->loc) {
                continue_flag = true;
                break;
            }
            next_loc.emplace_back(info->leg_info->path.to_index(next_point));
        }
        if (continue_flag) {
            continue;
        }
        for (auto &eu : info->round_info->enemy_units) {
            vector<double> scores;
            for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
                auto &next_point = eu.second->loc->next[d];
                if (d != DIRECTION::NONE && next_point == eu.second->loc) {
                    continue;
                }
                int enemy_loc = info->leg_info->path.to_index(next_point);
                if (find(now_loc.begin(), now_loc.end(), enemy_loc) != now_loc.end()) {
                    scores.emplace_back(10000);  // must be dead
                    continue;
                }
                int remain_loc_num = 0;
                for (int tmp_loc = 0; tmp_loc < info->leg_info->path.node_num; tmp_loc++) {
                    vector<double> distances;
                    double min_distance = numeric_limits<double>::max();
                    for (auto &nl : next_loc) {
                        double tmp = info->leg_info->path.get_cost(nl, tmp_loc);
                        if (tmp < min_distance) {
                            min_distance = tmp;
                        }
                    }
                    if (info->leg_info->path.get_cost(enemy_loc, tmp_loc) < min_distance) {
                        remain_loc_num++;
                    }
                }
                vector<double> distances;
                for (auto &nl : next_loc) {
                    distances.emplace_back(info->leg_info->path.get_cost(nl, enemy_loc));
                }
                double total_dis = accumulate(distances.begin(), distances.end(), 0.0);
                double score = 40000.0 / (5.0 + remain_loc_num) + 400.0 / (1.0 + total_dis);
                scores.emplace_back(score);
            }
            double min_score = *min_element(scores.begin(), scores.end());
            p.second = max({min_score, p.second});
        }
    }

    info->task_score->set_task_good_score(TASK_NAME::TaskEatEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
