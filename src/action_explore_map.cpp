#include "action_explore_map.h"

#include "player.h"

BT::NodeStatus ExploreMap::tick() {
    auto info = config().blackboard->get<Player *>("info");

    map<int, double> map_power;
    vector<double> env_score = info->game->env_score;
    for (int c = 0; c < 4; c++) {
        double max_score = -numeric_limits<double>::max();
        int max_score_loc = 0;
        for (int n = 0; n < info->leg_info->path.node_num; n++) {
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
            Point::Ptr point = info->leg_info->path.to_point(max_score_loc);
            for (auto &g : info->leg_info->vision_grids[point->index]) {
                env_score[g] = 0.0;
            }
        }
    }

    map<int, pair<int, int>> mu_time;
    for (const auto &mu : info->round_info->my_units) {
        mu_time[mu.first] = make_pair(0, mu.second->loc->index);
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

    vector<double> direction_score(info->task_score->score_num, 0.0);
    for (int idx = 0; idx < info->task_score->score_num; idx++) {
        map<int, DIRECTION> &map_direction = info->task_score->map_direction[idx];
        for (const auto &sds : single_direction_score) {
            if (map_direction[sds.mu_id] == sds.dir) {
                direction_score[idx] += sds.score;
            }
        }
    }

    info->task_score->set_task_good_score(TASK_NAME::TaskExploreMap, direction_score);
    return BT::NodeStatus::SUCCESS;
}
