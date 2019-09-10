#include "action_eat_power.h"

#include "player.h"

BT::NodeStatus EatPower::tick() {
    auto info = config().blackboard->get<Player *>("info");

    function<double(int start, int end, bool is_first_cloud)> get_cost;

    if (info->game->avoid_enemy) {
        get_cost = [info](int start, int end, bool is_first_cloud) -> double {
            return (double)info->game->get_cost(start, end);
        };
    } else {
        get_cost = [info](int start, int end, bool is_first_cloud) -> double {
            return (double)info->leg_info->path.get_cost(start, end, is_first_cloud);
        };
    }

    map<int, double> &map_power = info->game->map_power;

    map<int, pair<double, int>> mu_time;
    for (const auto &mu : info->round_info->my_units) {
        mu_time[mu.first] = make_pair(0, mu.second->loc->index);
    }
    map<int, int> mu_first_power;
    map<int, bool> map_cal;
    log_info("map_power size: %d", map_power.size());
    for (int i = 0; i < (int)map_power.size(); i++) {
        int mu_id = 0;
        double shortest_dis = numeric_limits<double>::max();
        int power_id = 0;
        for (const auto &power : map_power) {
            if (map_cal.find(power.first) != map_cal.end()) {
                continue;
            }
            for (const auto &mu : info->round_info->my_units) {
                double dis = (double)info->leg_info->path.get_cost(mu_time[mu.first].second, power.first, info->game->map_first_cloud[mu.first]) + mu_time[mu.first].first;
                if (dis < shortest_dis) {
                    shortest_dis = dis;
                    mu_id = mu.first;
                    power_id = power.first;
                }
            }
        }
        if (mu_first_power.count(mu_id) == 0) {
            mu_first_power[mu_id] = power_id;
            log_info("mu_id: %d  power_id: %d  power_score: %f", mu_id, power_id, map_power[power_id]);
        }
        map_cal[power_id] = true;
        mu_time[mu_id].first += shortest_dis;
        mu_time[mu_id].second = power_id;
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

    vector<double> direction_score(info->task_score->score_num, 0.0);
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        for (const auto &sds : single_direction_score) {
            if (map_direction[sds.mu_id] == sds.dir) {
                direction_score[idx] += sds.score;
            }
        }
    }

    info->task_score->set_task_score(TASK_NAME::TaskEatPower, direction_score);
    return BT::NodeStatus::SUCCESS;
}
