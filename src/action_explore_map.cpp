#include "action_explore_map.h"

#include "player.h"

BT::NodeStatus ExploreMap::tick() {
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

    map<int, double> map_power;
    vector<double> env_score = info->game->env_score;
    for (int c = 0; c < 4; c++) {
        double max_score = -numeric_limits<double>::max();
        int max_score_loc = 0;
        for (int n = 0; n < info->leg_info->path.node_num; n++) {
            Point::Ptr point = info->leg_info->path.to_point(n);
            if (point->wall || point->tunnel != DIRECTION::NONE) {
                continue;
            }
            double score = 0.0;
            for (auto &g : info->leg_info->vision_grids[n]) {
                score += env_score[g];
            }
            if (score > max_score) {
                max_score = score;
                max_score_loc = n;
            }
        }
        if (!equal_double(max_score, 1e-6)) {
            map_power[max_score_loc] = max_score;
            log_info("max_score_loc: %d max_score: %f", max_score_loc, max_score);
            Point::Ptr point = info->leg_info->path.to_point(max_score_loc);
            for (auto &g : info->leg_info->vision_grids[point->index]) {
                env_score[g] = 0.0;
            }
        }
    }

//    map<int, pair<double, int>> mu_time;
//    for (const auto &mu : info->round_info->my_units) {
//        mu_time[mu.first] = make_pair(0, mu.second->loc->index);
//    }
    map<int, int> mu_first_power;
    for (const auto &power : map_power) {
        int mu_id = -1;
        int power_id = 0;
        double shortest_dis = numeric_limits<double>::max();
        for (const auto &mu : info->round_info->my_units) {
            if (mu_first_power.count(mu.first) > 0) {
                continue;
            }
            double dis = get_cost(mu.second->loc->index, power.first, true);
            if (dis < shortest_dis) {
                shortest_dis = dis;
                mu_id = mu.first;
                power_id = power.first;
            }
        }
        if (mu_id != -1 && mu_first_power.count(mu_id) == 0) {
            mu_first_power[mu_id] = power.first;
            log_info("mu_id: %d  map_power_id: %d  map_power_score: %f", mu_id, power_id, map_power[power_id]);
        }
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
        Point::Ptr p = info->round_info->my_units[mu.first]->loc;
        for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT}) {
            double cost_a = get_cost(p->index, p->next[d]->index, true);
            double cost_b = get_cost(p->next[d]->index, mu.second, true);
            double score = map_power[mu.second] / (max(1.0, cost_a) + cost_b + 1);
            single_direction_score.emplace_back(dir_score(mu.first, d, score));
            log_info("mu_id: %d, dir: %d cost_a: %f cost_b: %f score:%f", mu.first, d, cost_a, cost_b, score);
        }
    }

    vector<double> direction_score(info->task_score->score_num, 0.0);
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        for (const auto &sds : single_direction_score) {
            if (map_direction[sds.mu_id] == sds.dir) {
                direction_score[idx] += sds.score / (info->leg_info->vision * 5);
            }
        }
    }

    info->task_score->set_task_score(TASK_NAME::TaskExploreMap, direction_score);
    return BT::NodeStatus::SUCCESS;
}
