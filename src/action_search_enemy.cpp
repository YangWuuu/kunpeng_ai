#include "action_search_enemy.h"

#include "player.h"

BT::NodeStatus SearchEnemy::tick() {
    auto info = config().blackboard->get<Player *>("info");

    if (!info->game->search_enemy) {
        return BT::NodeStatus::SUCCESS;
    }
    
    map<int, double> map_danger;
    vector<double> all_danger = info->game->all_danger;
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
                score += all_danger[g];
            }
            if (score > max_score) {
                max_score = score;
                max_score_loc = n;
            }
        }
        if (!equal_double(max_score, 1e-6)) {
            map_danger[max_score_loc] = max_score;
            log_info("max_score_loc: %d max_score: %f", max_score_loc, max_score);
            Point::Ptr point = info->leg_info->path.to_point(max_score_loc);
            for (auto &g : info->leg_info->vision_grids[point->index]) {
                all_danger[g] = 0.0;
            }
        }
    }

    map<int, int> mu_first_danger;
    for (const auto &power : map_danger) {
        int mu_id = -1;
        int danger_id = 0;
        double shortest_dis = numeric_limits<double>::max();
        for (const auto &mu : info->round_info->my_units) {
            if (mu_first_danger.count(mu.first) > 0) {
                continue;
            }
            double dis = info->leg_info->path.get_cost(mu.second->loc->index, power.first, true);
            if (dis < shortest_dis) {
                shortest_dis = dis;
                mu_id = mu.first;
                danger_id = power.first;
            }
        }
        if (mu_id != -1 && mu_first_danger.count(mu_id) == 0) {
            mu_first_danger[mu_id] = power.first;
            log_info("mu_id: %d  danger_id: %d  map_danger_score: %f", mu_id, danger_id, map_danger[danger_id]);
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
    for (const auto &mu : mu_first_danger) {
        Point::Ptr p = info->round_info->my_units[mu.first]->loc;
        for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT}) {
            double cost = info->leg_info->path.get_cost(p->next[d]->index, mu.second, true);
            double score = map_danger[mu.second] / (cost + 1);
            single_direction_score.emplace_back(dir_score(mu.first, d, score));
            log_info("mu_id: %d, dir: %d cost: %f score:%f", mu.first, d, cost, score);
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

    info->task_score->set_task_score(TASK_NAME::TaskSearchEnemy, direction_score);
    return BT::NodeStatus::SUCCESS;
}
