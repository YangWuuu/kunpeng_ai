#ifndef AI_YANG_GAME_H
#define AI_YANG_GAME_H

#include <vector>
#include <set>

#include "util.h"

using namespace std;

class Game : public enable_shared_from_this<Game> {
public:
    explicit Game(shared_ptr<LegStartInfo> &_leg) : leg_info(_leg), round_id(0), is_eat(false) {
        power_score = vector<double>(leg_info->path.node_num, 0.0);
        env_score_limit = vector<double>(leg_info->path.node_num, 0.0);
        visit_time = vector<int>(leg_info->path.node_num, 0);
        for (int n = 0; n < leg_info->path.node_num; n++) {
            Point::Ptr point = leg_info->path.to_point(n);
            if (!point->wall && point->tunnel == DIRECTION::NONE && !point->wormhole) {
                env_score_limit[n] = 1.0 / (my_pow(2 * leg_info->vision + 1, 2));
            }
        }
        env_score = env_score_limit;
        if (leg_info->enemy_team.units.size() != 4) {
            log_error("enemy_team size is not 4: %d", leg_info->enemy_team.units.size());
        }
        for (int i = 0 ; i < (int)leg_info->enemy_team.units.size(); i++) {
            enemy_units_map[leg_info->enemy_team.units[i]] = i;
        }
        vec_danger.resize(leg_info->enemy_team.units.size(), vector<double>(leg_info->path.node_num, 100.0));
    }

    void update_round_info(const shared_ptr<RoundInfo> &round_info);

    bool isEnd() { return round_id >= 300; }

    double get_cost(int start, int end);

private:
    void update_score();
    void update_remain_life();
    void update_map_power();
    void update_danger();
    void update_first_cloud();

    void update_dist();
    void predict_enemy();

    void SPFA(int k);
    bool relax(int u, int v, vector<double> &d);
public:
    shared_ptr<LegStartInfo> leg_info;

    map<int, Unit::Ptr> my_units;
    map<int, Unit::Ptr> enemy_units;
    int round_id;
    bool is_eat;
    vector<double> power_score;
    vector<double> env_score;
    vector<double> env_score_limit;
    vector<int> visit_time;

    map<int, double> map_power;

    vector<shared_ptr<RoundInfo>> vec_round_info;
    int enemy_all_remain_life{};
    int my_all_remain_life{};

    map<int, int> enemy_units_map;
    vector<vector<double>> vec_danger;
    vector<double> all_danger;

    map<int, bool> map_first_cloud;

    bool eat_enemy{};
    bool run_away{};
    bool search_enemy{};
    bool avoid_enemy{};
    bool out_vision{};

    bool all_enemy_in_vision{};

    vector<bool> danger_in_vision;
    vector<bool> danger_eat_in_vision;

    map<int, bool> map_enemy_predict;
    map<int, int> map_enemy_loc;
    map<int, int> map_enemy_repeat;

private:
    set<int> see_alive_enemy;
    set<int> dead_enemy;

    vector<bool> is_cal;
    const double inf = 1e10;
    vector<vector<double>> G;
    vector<vector<double>> dist;
};

#endif //AI_YANG_GAME_H
