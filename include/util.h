#ifndef AI_YANG_UTIL_H
#define AI_YANG_UTIL_H

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <numeric>
#include <random>

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
};

const static vector<string> TASK_NAME_STRING = {
        "TaskEatEnemy",
        "TaskExploreMap",
        "TaskEatPower",
        "TaskRunAway"
};

enum DIRECTION {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    NONE,
};

const static  vector<string> DIRECTION_STRING = {
        "UP   ",
        "DOWN ",
        "LEFT ",
        "RIGHT",
        "NONE "
};

class Point {
public:
    using Ptr = shared_ptr<Point>;

    static Point::Ptr gen(int _x, int _y) {
        return make_shared<Point>(_x, _y);
    }

    static int distance(const Ptr &p1, const Ptr &p2) {
        return abs(p1->x - p2->x) + abs(p1->y - p2->y);
    }

    static int max_xy(const Ptr &p1, const Ptr &p2) {
        return max(abs(p1->x - p2->x), abs(p1->y - p2->y));
    }

    explicit Point(int _x, int _y) : x(_x), y(_y), tunnel(DIRECTION::NONE), wall(false), wormhole(nullptr),
                                     visited(false) {
        next[DIRECTION::UP] = nullptr;
        next[DIRECTION::DOWN] = nullptr;
        next[DIRECTION::LEFT] = nullptr;
        next[DIRECTION::RIGHT] = nullptr;
        next[DIRECTION::NONE] = nullptr;
    }

public:
    int x;
    int y;
    DIRECTION tunnel;
    bool wall;
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
    void setMap(map<int, map<int, Point::Ptr>> *_maps, int _width, int _height) {
        maps = _maps;
        width = _width;
        height = _height;
        node_num = width * height;
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
                    G[i][j] = 1;
                } else if (i == j) {
                    G[i][j] = 0;
                }
            }
        }
        dist = G;
        Floyd();
    }

    int get_cost(int start, int end) {
        return dist[start][end];
    }

    int get_cost(const Point::Ptr &start, int end) {
        return dist[to_index(start)][end];
    }

    int get_cost(int start, const Point::Ptr &end) {
        return dist[start][to_index(end)];
    }

    int get_cost(const Point::Ptr &start, const Point::Ptr &end) {
        return dist[to_index(start)][to_index(end)];
    }

    DIRECTION find_next_direction(const Point::Ptr &start, const Point::Ptr &end) {
        DIRECTION ret = DIRECTION::NONE;
        int i = to_index(start);
        int j = to_index(end);
        for (auto &dp : start->next) {
            if (dp.second == to_point(path[i][j])) {
                ret = dp.first;
                break;
            }
        }
        return ret;
    }

public:
    Point::Ptr to_point(int index) {
        return (*maps)[index % width][index / width];
    }

    int to_index(const Point::Ptr &point) {
        return point->x + point->y * width;
    }

    int to_index(int x, int y) {
        return x + y * width;
    }

    int node_num{};
private:
    void Floyd() {
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

    map<int, map<int, Point::Ptr>> *maps{};
    int height{};
    int width{};

    const int inf = 0xffff;
    vector<vector<int>> G;
    vector<vector<int>> dist;
    vector<vector<int>> path;
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
        for (int i = 0; i < width; i++) {
            map<int, Point::Ptr> tmp;
            for (int j = 0; j < height; j++) {
                tmp[j] = Point::gen(i, j);
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
        path.setMap(&maps, width, height);
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
        } else if (next_p->next[DIRECTION::NONE] != next_p) {
            for (Point::Ptr &q : vec_p) {
                q->next[DIRECTION::NONE] = next_p->next[DIRECTION::NONE];
            }
            return next_p->next[DIRECTION::NONE];
        } else {
            // TODO loop
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
        my_units_map.clear();
        my_units_count = 0;
        good_score.clear();
        bad_score.clear();
        for (auto &mu : my_units) {
            my_units_map[mu.first] = my_units_count;
            my_map_units[my_units_count] = mu.first;
            my_units_count++;
        }
        score_num = my_pow(5, my_units_count);
    }

    void set_task_good_score(TASK_NAME task_name, vector<pair<map<int, DIRECTION>, double>> &direction_score) {
        set_task_score(good_score, task_name, direction_score);
    }

    void set_task_bad_score(TASK_NAME task_name, vector<pair<map<int, DIRECTION>, double>> &direction_score) {
        set_task_score(bad_score, task_name, direction_score);
    }

    void set_task_score(map<TASK_NAME, vector<double>> &score, TASK_NAME task_name,
                        vector<pair<map<int, DIRECTION>, double>> &direction_score) {
        if (score.count(task_name) == 0) {
            score[task_name] = vector<double>(score_num, 0.0);
        }
        for (auto &ds : direction_score) {
            int id = 0;
            for (auto &unit_direction : ds.first) {
                id += my_pow(5, my_units_map[unit_direction.first]) * (int) unit_direction.second;
            }
            score[task_name][id] = ds.second;
        }
    }

    map<int, DIRECTION> get_map_direction(int id) {
        map<int, DIRECTION> ret;
        for (int i = 0; i < my_units_count; i++) {
            ret[my_map_units[i]] = DIRECTION(id / (my_pow(5, i)) % 5);
        }
        return ret;
    }

public:
    int my_units_count;
    map<int, int> my_units_map;
    map<int, int> my_map_units;
    int score_num;

    map<TASK_NAME, vector<double>> good_score;
    map<TASK_NAME, vector<double>> bad_score;
};

inline bool
in_vision(const map<int, Unit::Ptr> &my_units, const Point::Ptr &loc, const shared_ptr<LegStartInfo> &leg_info) {
    bool ret = false;
    for (auto &mu : my_units) {
        if (Point::distance(mu.second->loc, loc) <= leg_info->vision) {
            ret = true;
            break;
        }
    }
    return ret;
}

inline bool equal_double(double a, double b) {
    return abs(a - b) < 1e-6;
}

inline vector<pair<int, int>> get_vision_grids(int x, int y, int width, int height, int vision) {
    vector<pair<int, int>> ret;
    for (int i = max(x - vision, 0); i <= min(x + vision, width - 1); i++) {
        for (int j = max(y - vision, 0); j <= min(y + vision, height - 1); j++) {
            ret.emplace_back(make_pair(i, j));
        }
    }
    return ret;
};

inline vector<pair<int, int>> get_vision_grids(Point::Ptr &loc, const shared_ptr<LegStartInfo>& leg_info) {
    return get_vision_grids(loc->x, loc->y, leg_info->width, leg_info->height, leg_info->vision);
};

inline vector<pair<int, int>> get_vision_grids(Point::Ptr &loc, const shared_ptr<LegStartInfo>& leg_info, int vision) {
    return get_vision_grids(loc->x, loc->y, leg_info->width, leg_info->height, vision);
};

inline vector<pair<int, int>> get_vision_grids(int x, int y, const shared_ptr<LegStartInfo>& leg_info, int vision) {
    return get_vision_grids(x, y, leg_info->width, leg_info->height, vision);
};

template<typename T>
inline vector<int> sort_indexes(const std::vector<T> &v) {
    vector<int> idx(v.size());
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&v](int i1, int i2) { return v[i1] < v[i2]; });
    return idx;
}

#endif //AI_YANG_UTIL_H
