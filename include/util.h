#ifndef AI_YANG_UTIL_H
#define AI_YANG_UTIL_H

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <numeric>
#include <random>
#include <sstream>
#include <algorithm>

#include "log.h"

using namespace std;

static default_random_engine e(2019);

inline vector<string> string_split(const string &s, const string &c) {
    vector<string> v;
    string::size_type pos1, pos2;
    size_t len = s.length();
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
        v.emplace_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != len)
        v.emplace_back(s.substr(pos1));
    return v;
}

enum TASK_NAME {
    TaskEatEnemy = 0,
    TaskExploreMap,
    TaskEatPower,
    TaskRunAway,
    TaskSearchEnemy,
    TaskAvoidEnemy,
    TaskOutVision,
    TaskRemoveInvalid,
};

const static vector<string> TASK_NAME_STRING = {
        "TaskEatEnemy",
        "TaskExploreMap",
        "TaskEatPower",
        "TaskRunAway",
        "TaskSearchEnemy",
        "TaskAvoidEnemy",
        "TaskOutVision",
        "TaskRemoveInvalid",
};

enum DIRECTION {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    NONE,
};

const static vector<string> DIRECTION_STRING = {
        "UP   ",
        "DOWN ",
        "LEFT ",
        "RIGHT",
        "NONE "
};

class Point : public enable_shared_from_this<Point> {
public:
    using Ptr = shared_ptr<Point>;

    static Point::Ptr gen(int _x, int _y, int _index) {
        return make_shared<Point>(_x, _y, _index);
    }

    static int distance(const Ptr &p1, const Ptr &p2) {
        return abs(p1->x - p2->x) + abs(p1->y - p2->y);
    }

    static int max_xy(const Ptr &p1, const Ptr &p2) {
        return max(abs(p1->x - p2->x), abs(p1->y - p2->y));
    }

    explicit Point(int _x, int _y, int _index) :
            x(_x), y(_y), index(_index), tunnel(DIRECTION::NONE), wall(false), cloud(false), wormhole(nullptr), visited(false) {
        next[DIRECTION::UP] = nullptr;
        next[DIRECTION::DOWN] = nullptr;
        next[DIRECTION::LEFT] = nullptr;
        next[DIRECTION::RIGHT] = nullptr;
        next[DIRECTION::NONE] = nullptr;
    }

    Ptr next_point(DIRECTION d, bool is_first_cloud=true) {
        if (cloud && is_first_cloud) {
            return shared_from_this();
        } else {
            return next[d];
        }
    }

public:
    int x;
    int y;
    int index;
    DIRECTION tunnel;
    bool wall;
    bool cloud;
    Ptr wormhole;
    map<DIRECTION, Ptr> next;

    // check loop
    bool visited;
    //print
    string name;
};

class Unit {
public:
    using Ptr = std::shared_ptr<Unit>;

    static Unit::Ptr gen(int _id, int _score, int _sleep, int _team, const Point::Ptr &_loc) {
        return make_shared<Unit>(_id, _score, _sleep, _team, _loc);
    }

    Unit(int _id, int _score, int _sleep, int _team, Point::Ptr _loc) :
            id(_id), score(_score), sleep(_sleep), team(_team) {
        direction = DIRECTION::NONE;
        loc = move(_loc);
    }

public:
    int id;
    int score;
    int sleep;
    int team;
    DIRECTION direction;
    Point::Ptr loc;
};

class Power {
public:
    Power(int _point, Point::Ptr &_loc) : point(_point), loc(_loc) {}

public:
    int point;
    Point::Ptr loc;
};

class Team {
public:
    Team() : id(0) {}

public:
    int id;
    string force;
    vector<int> units;
};

class Path {
public:
    void setWidthHeight(int _width, int _height, int _vision) {
        width = _width;
        height = _height;
        vision = _vision;
        node_num = width * height;
    }

    void setMap(map<int, map<int, Point::Ptr>> *_maps) {
        maps = _maps;
        vec_point.clear();
        for (int index = 0; index < node_num; index++) {
            vec_point.emplace_back((*maps)[index % width][index / width]);
        }
        path = vector<vector<int>>(node_num, vector<int>(node_num, 0));
        for (int i = 0; i < node_num; i++) {
            for (int j = 0; j < node_num; j++) {
                path[i][j] = j;
            }
        }
        G = vector<vector<int>>(node_num, vector<int>(node_num, inf));
        for (int i = 0; i < node_num; i++) {
            for (auto &dp : to_point(i)->next) {
                int j = to_index(dp.second);
                if (i != j) {
                    if (to_point(i)->cloud) {
                        G[i][j] = 2;
                    } else {
                        G[i][j] = 1;
                    }
                } else if (i == j) {
                    G[i][j] = 0;
                }
            }
        }
        Floyd();
//        SPFA();
        cal_danger_index();
    }

    int get_cost(int start, int end, bool is_first_cloud=true) {
        if (!is_first_cloud && to_point(start)->cloud && start != end) {
            return dist[start][end] - 1;
        } else {
            return dist[start][end];
        }
    }

    int get_cost(const Point::Ptr &start, int end, bool is_first_cloud=true) {
        return get_cost(to_index(start), end, is_first_cloud);
    }

    int get_cost(int start, const Point::Ptr &end, bool is_first_cloud=true) {
        return get_cost(start, to_index(end), is_first_cloud);
    }

    int get_cost(const Point::Ptr &start, const Point::Ptr &end, bool is_first_cloud=true) {
        return get_cost(to_index(start), to_index(end), is_first_cloud);
    }

public:
    Point::Ptr to_point(int index) {
        return vec_point[index];
    }

    int to_index(int x, int y) {
        return x + y * width;
    }

    static void vec_intersection(const vector<int> &vec_a, const vector<int> &vec_b, vector<int> &index_vec) {
        auto first_a = vec_a.begin();
        auto first_b = vec_b.begin();
        while (first_a != vec_a.end() && first_b != vec_b.end()) {
            if (*first_a < *first_b)
                ++first_a;
            else if (*first_a > *first_b)
                ++first_b;
            else
            {
                index_vec.emplace_back(*first_a);
                ++first_a;
                ++first_b;
            }
        }
    }

    vector<int> get_intersection1(int attack_index1, int defence_index) {
        int key1 = attack_index1 * node_num + defence_index;
        auto it1 = map_intersection_index1.find(key1);
        if (it1 != map_intersection_index1.end()) {
            return it1->second;
        }
        vector<int> &reach_index_vec = map_intersection_index1[key1];
        for (int tmp_index = 0; tmp_index < node_num; tmp_index++) {
            if (get_cost(defence_index, tmp_index) < get_cost(attack_index1, tmp_index)) {
                reach_index_vec.emplace_back(tmp_index);
            }
        }
        return reach_index_vec;
    }

    vector<int> get_intersection2(int attack_index1, int attack_index2, int defence_index) {
        int key1 = attack_index1 * node_num + defence_index;
        int key2 = attack_index2 * node_num + defence_index;
        auto it1 = map_intersection_index2.find(key1);
        if (it1 != map_intersection_index2.end()) {
            auto it2 = it1->second.find(key2);
            if (it2 != it1->second.end()) {
                return it2->second;
            }
        }
        vector<int> &reach_index_vec = map_intersection_index2[key1][key2];
        const vector<int> &reach_index_vec_a = get_intersection1(attack_index1, defence_index);
        const vector<int> &reach_index_vec_b = get_intersection1(attack_index2, defence_index);
        vec_intersection(reach_index_vec_a, reach_index_vec_b, reach_index_vec);
        return reach_index_vec;
    }

    vector<int> get_intersection3(int attack_index1, int attack_index2, int attack_index3, int defence_index) {
        int key1 = attack_index1 * node_num + defence_index;
        int key2 = attack_index2 * node_num + defence_index;
        int key3 = attack_index3 * node_num + defence_index;
        auto it1 = map_intersection_index3.find(key1);
        if (it1 != map_intersection_index3.end()) {
            auto it2 = it1->second.find(key2);
            if (it2 != it1->second.end()) {
                auto it3 = it2->second.find(key3);
                if (it3 != it2->second.end()) {
                    return it3->second;
                }
            }
        }
        vector<int> &reach_index_vec = map_intersection_index3[key1][key2][key3];
        const vector<int> &reach_index_vec_a = get_intersection1(attack_index1, defence_index);
        const vector<int> &reach_index_vec_b = get_intersection2(attack_index2, attack_index3, defence_index);
        vec_intersection(reach_index_vec_a, reach_index_vec_b, reach_index_vec);
        return reach_index_vec;
    }

    vector<int> get_intersection4(int attack_index1, int attack_index2, int attack_index3, int attack_index4, int defence_index) {
        int key1 = attack_index1 * node_num + defence_index;
        int key2 = attack_index2 * node_num + defence_index;
        int key3 = attack_index3 * node_num + defence_index;
        int key4 = attack_index3 * node_num + defence_index;
        auto it1 = map_intersection_index4.find(key1);
        if (it1 != map_intersection_index4.end()) {
            auto it2 = it1->second.find(key2);
            if (it2 != it1->second.end()) {
                auto it3 = it2->second.find(key3);
                if (it3 != it2->second.end()) {
                    auto it4 = it3->second.find(key4);
                    if (it4 != it3->second.end()) {
                        return it4->second;
                    }
                }
            }
        }
        vector<int> &reach_index_vec = map_intersection_index4[key1][key2][key3][key4];
        const vector<int> &reach_index_vec_a = get_intersection2(attack_index1, attack_index2, defence_index);
        const vector<int> &reach_index_vec_b = get_intersection2(attack_index3, attack_index4, defence_index);
        vec_intersection(reach_index_vec_a, reach_index_vec_b, reach_index_vec);
        return reach_index_vec;
    }

    vector<int> get_intersection(const vector<int> &attack_index_vec, int defence_index) {
        if (attack_index_vec.empty()) {
            log_error("attack_index_vec is empty");
            return vector<int>();
        } else if (attack_index_vec.size() == 1) {
            return get_intersection1(attack_index_vec[0], defence_index);
        } else if (attack_index_vec.size() == 2) {
            return get_intersection2(attack_index_vec[0], attack_index_vec[1], defence_index);
        } else if (attack_index_vec.size() == 3) {
            return get_intersection3(attack_index_vec[0], attack_index_vec[1], attack_index_vec[2], defence_index);
        } else if (attack_index_vec.size() == 4) {
            return get_intersection4(attack_index_vec[0], attack_index_vec[1], attack_index_vec[2], attack_index_vec[3], defence_index);
        } else {
            log_error("attack_index_vec is too large: %d", attack_index_vec.size());
            return vector<int>();
        }
    }

    int get_intersection_size(const vector<int> &attack_index_vec, int defence_index) {
        int remain_loc_num = 0;
        for (int tmp_loc = 0; tmp_loc < node_num; tmp_loc++) {
            int enemy_dis = get_cost(defence_index, tmp_loc);
            bool in_reach = false;
            for (auto& nl : attack_index_vec) {
                int tmp_dis = get_cost(nl, tmp_loc);
                if (tmp_dis <= enemy_dis) {
                    in_reach = true;
                    break;
                }
            }
            if (!in_reach) {
                remain_loc_num++;
            }
        }
        return remain_loc_num;
    }

    int get_intersection_size2(const vector<int> &attack_index_vec, int defence_index) {
        const vector<int> &intersection = get_intersection(attack_index_vec, defence_index);
        return (int)intersection.size();
    }

    int node_num{};
    vector<vector<int>> G;

    vector<bool> is_danger_index;
    vector<bool> is_eat_danger_index;

private:
    int to_index(const Point::Ptr &point) {
        return point->x + point->y * width;
    }

    void Floyd() {
        dist = G;
        for (int k = 0; k < node_num; k++) {
            for (int i = 0; i < node_num; i++) {
                for (int j = 0; j < node_num; j++) {
                    if ((dist[i][k] + dist[k][j] < dist[i][j]) && (dist[i][k] != inf) && (dist[k][j] != inf) &&
                        (i != j)) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        path[i][j] = path[i][k];
                    }
                }
            }
        }
    }

    void SPFA() {
        dist.assign(node_num, vector<int>(node_num, inf));
        for (int k = 0; k < node_num; k++) {
            Point::Ptr point = to_point(k);
            if (point->wall || point->tunnel != DIRECTION::NONE) {
                continue;
            }
            queue<int> Q;
            vector<bool> is_in_Q(node_num, false);

            Q.push(k);
            is_in_Q[k] = true;
            dist[k][k] = 0;
            while (!Q.empty()) {
                int u = Q.front();
                Q.pop();
                is_in_Q[u] = false;
                for (int v = 0; v < node_num; ++v) {
                    if (relax(u, v, dist[k]) && !is_in_Q[v]) {
                        Q.push(v);
                        is_in_Q[v] = true;
                    }
                }
            }
        }
    }

    inline bool relax(int u, int v, vector<int> &d) {
        int nlen = d[u] + G[u][v];
        if (nlen < d[v]) {
            d[v] = nlen;
            return true;
        }
        return false;
    }

    void cal_danger_index() {
        is_danger_index.assign(node_num, false);
        is_eat_danger_index.assign(node_num, false);
        for (int index = 0; index < node_num; index++) {
            Point::Ptr p = to_point(index);
            for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
                if (Point::max_xy(p, p->next[d]) > vision) {
                    is_danger_index[p->next[d]->index] = true;
                    for (DIRECTION dd : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT, DIRECTION::NONE}) {
                        is_eat_danger_index[p->next[d]->next[dd]->index] = true;
                    }
                }
            }
        }
    }

    map<int, map<int, Point::Ptr>> *maps{};
    int height{};
    int width{};
    int vision{};

    const int inf = 0xffff;

    vector<vector<int>> dist;
    vector<vector<int>> path;

    vector<Point::Ptr> vec_point;

    map<int, vector<int>> map_intersection_index1;
    map<int, map<int, vector<int>>> map_intersection_index2;
    map<int, map<int, map<int, vector<int>>>> map_intersection_index3;
    map<int, map<int, map<int, map<int, vector<int>>>>> map_intersection_index4;
};

class RoundInfo {
public:
    RoundInfo() : round_id(-1), my_point(-1), enemy_point(-1), my_remain_life(-1), enemy_remain_life(-1) {}

public:
    string mode;
    int round_id;
    map<int, Unit::Ptr> my_units;
    map<int, Unit::Ptr> enemy_units;
    vector<Power> powers;
    int my_point;
    int enemy_point;
    int my_remain_life;
    int enemy_remain_life;
};

class LegStartInfo {
public:
    LegStartInfo() : width(0), height(0), vision(0) {}

    void GenMap(int w, int h, int v) {
        maps.clear();
        width = w;
        height = h;
        vision = v;
        path.setWidthHeight(width, height, vision);
        for (int i = 0; i < width; i++) {
            map<int, Point::Ptr> tmp;
            for (int j = 0; j < height; j++) {
                tmp[j] = Point::gen(i, j, path.to_index(i, j));
            }
            maps[i] = move(tmp);
        }
    }

    void construct_map() {
        for (auto &col : maps) {
            for (auto &r : col.second) {
                Point::Ptr &p = r.second;
                p->next[DIRECTION::NONE] = find_next_point_with_no_move(p);
            }
        }
        for (auto &col : maps) {
            for (auto &r : col.second) {
                Point::Ptr &p = r.second;
                for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT}) {
                    Point::Ptr next_p = around_with_meteor_one_step(p, d);
                    p->next[d] = next_p->next[DIRECTION::NONE];
                    if (p != next_p && next_p->wormhole) {
                        p->next[d] = next_p->wormhole;
                    }
                }
            }
        }
        path.setMap(&maps);
        cal_vision_grids();
    }

    Point::Ptr around_with_meteor_one_step(const Point::Ptr &p, DIRECTION d) {
        int x = p->x;
        int y = p->y;
        int next_x = x;
        int next_y = y;
        switch (d) {
            case DIRECTION::UP:
                if (y > 0)
                    next_y--;
                break;
            case DIRECTION::DOWN:
                if (y < height - 1)
                    next_y++;
                break;
            case DIRECTION::LEFT:
                if (x > 0)
                    next_x--;
                break;
            case DIRECTION::RIGHT:
                if (x < width - 1)
                    next_x++;
                break;
            case DIRECTION::NONE:
                break;
            default:
                break;
        }
        if (maps[next_x][next_y]->wall || x + y == next_x + next_y)
            return p;
        return maps[next_x][next_y];
    }

    Point::Ptr find_next_point_with_no_move(const Point::Ptr &p) {
        if (p->visited) {
            return p->next[DIRECTION::NONE];
        }
        p->visited = true;
        if (p->tunnel == DIRECTION::NONE) {
            p->next[DIRECTION::NONE] = p;
            return p;
        }
        vector<Point::Ptr> vec_p;
        Point::Ptr next_p = p;
        do {
            vec_p.emplace_back(next_p);
            next_p->visited = true;
            next_p = around_with_meteor_one_step(next_p, next_p->tunnel);
        } while (next_p->tunnel != DIRECTION::NONE && !next_p->visited);
        if (next_p->tunnel == DIRECTION::NONE ||
            around_with_meteor_one_step(next_p, next_p->tunnel) == next_p) {
            for (Point::Ptr &q : vec_p) {
                q->next[DIRECTION::NONE] = next_p;
            }
            return next_p;
        } else {
            for (Point::Ptr &q : vec_p) {
                q->next[DIRECTION::NONE] = next_p->next[DIRECTION::NONE];
            }
            return next_p->next[DIRECTION::NONE];
        }
    }

    void cal_vision_grids() {
        vision_grids = vector<vector<int>>(path.node_num, vector<int>());
        vision_grids_1 = vector<vector<int>>(path.node_num, vector<int>());
        vision_grids_3 = vector<vector<int>>(path.node_num, vector<int>());
        vision_grids_cross = vector<vector<int>>(path.node_num, vector<int>());
        for (int n = 0; n < path.node_num; n++) {
            auto p = path.to_point(n);
            int x = p->x;
            int y = p->y;
            for (int i = max(x - vision, 0); i <= min(x + vision, width - 1); i++) {
                for (int j = max(y - vision, 0); j <= min(y + vision, height - 1); j++) {
                    vision_grids[n].emplace_back(path.to_index(i, j));
                }
            }
            for (int i = max(x - 1, 0); i <= min(x + 1, width - 1); i++) {
                for (int j = max(y - 1, 0); j <= min(y + 1, height - 1); j++) {
                    vision_grids_3[n].emplace_back(path.to_index(i, j));
                }
            }
            for (DIRECTION d : {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::LEFT, DIRECTION::RIGHT}) {
                vision_grids_1[n].emplace_back(path.to_point(n)->next[d]->index);
            }
            vision_grids_cross.emplace_back(path.to_index(x, y));
            vision_grids_cross.emplace_back(path.to_index(max(x - 1, 0), y));
            vision_grids_cross.emplace_back(path.to_index(min(x + 1, width), y));
            vision_grids_cross.emplace_back(path.to_index(x, max(y - 1, 0)));
            vision_grids_cross.emplace_back(path.to_index(x, min(y + 1, height)));
        }
    }

public:
    int width;
    int height;
    int vision;
    map<int, map<int, Point::Ptr>> maps;
    Path path;
    Team my_team;
    Team enemy_team;
    vector<vector<int>> vision_grids;
    vector<vector<int>> vision_grids_3;
    vector<vector<int>> vision_grids_1;
    vector<vector<int>> vision_grids_cross;
};

inline int my_pow(int base, int exp) {
    int ret = 1;
    while (exp > 0) {
        ret *= base;
        exp--;
    }
    return ret;
}

class TaskScore {
public:
    void init_every_round(const map<int, Unit::Ptr> &my_units) {
        score.clear();
        int now_units_id = 0;
        for (auto &mu : my_units) {
            now_units_id += my_pow(8, mu.first);
        }
        if (now_units_id != last_units_id) {
            last_units_id = now_units_id;
            my_units_map.clear();
            my_map_units.clear();
            my_units_count = 0;
            for (auto &mu : my_units) {
                my_units_map[mu.first] = my_units_count;
                my_map_units[my_units_count] = mu.first;
                my_units_count++;
            }
            score_num = my_pow(5, my_units_count);
            if (my_units_count == 0) {
                score_num = 0;
            }
            get_map_direction();
        }
    }

    void set_task_score(TASK_NAME task_name, vector<double> &direction_score) {
        score[task_name] = move(direction_score);
    }

    void get_map_direction() {
        map_direction.clear();
        for (int id = 0; id < score_num; id++) {
            map<int, DIRECTION> ret;
            for (int i = 0; i < my_units_count; i++) {
                ret[my_map_units[i]] = DIRECTION(id / (my_pow(5, i)) % 5);
            }
            map_direction.emplace_back(ret);
        }
    }

public:
    int my_units_count;
    map<int, int> my_units_map;
    map<int, int> my_map_units;
    int score_num;

    vector<map<int, DIRECTION>> map_direction;

    map<TASK_NAME, vector<double>> score;

private:
    int last_units_id{-1};
};

inline bool equal_double(double a, double b) {
    return abs(a - b) < 1e-6;
}

template<typename T>
inline vector<int> sort_indexes(const std::vector<T> &v) {
    vector<int> idx(v.size());
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&v](int i1, int i2) { return v[i1] < v[i2]; });
    return idx;
}

#endif //AI_YANG_UTIL_H
