#ifndef AI_YANG_UTIL_H
#define AI_YANG_UTIL_H

#include <map>
#include <string>
#include <memory>
#include <vector>
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

inline vector<pair<int, int>> getVisionGrids(int x, int y, int width, int height, int vision) {
    vector<pair<int, int>> ret;
    for (int i = max(x - vision, 0); i <= min(x + vision, width - 1); i++) {
        for (int j = max(y - vision, 0); j <= min(y + vision, height - 1); j++) {
            ret.emplace_back(make_pair(i, j));
        }
    }
    return ret;
};

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE,
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

    explicit Point(int _x, int _y) : x(_x), y(_y), tunnel(Direction::NONE), wall(false), wormhole(nullptr),
                                     visited(false) {
        next[Direction::UP] = nullptr;
        next[Direction::DOWN] = nullptr;
        next[Direction::LEFT] = nullptr;
        next[Direction::RIGHT] = nullptr;
        next[Direction::NONE] = nullptr;
    }

public:
    int x;
    int y;
    Direction tunnel;
    bool wall;
    Ptr wormhole;
    map<Direction, Ptr> next;

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
        direction = Direction::NONE;
        loc = move(_loc);
    }

public:
    int id;
    int score;
    int sleep;
    int team;
    Direction direction;
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
                p->next[Direction::NONE] = find_next_point_with_no_move(p);
            }
        }
        for (auto &col : maps) {
            for (auto &r : col.second) {
                Point::Ptr &p = r.second;
                for (Direction d : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT}) {
                    Point::Ptr next_p = around_with_meteor_one_step(p, d);
                    p->next[d] = next_p->next[Direction::NONE];
                    if (p != next_p && next_p->wormhole) {
                        p->next[d] = next_p->wormhole;
                    }
                }
            }
        }
    }

    Point::Ptr around_with_meteor_one_step(const Point::Ptr &p, Direction d) {
        int x = p->x;
        int y = p->y;
        int next_x = x;
        int next_y = y;
        switch (d) {
            case Direction::UP:
                if (y > 0)
                    next_y--;
                break;
            case Direction::DOWN:
                if (y < height - 1)
                    next_y++;
                break;
            case Direction::LEFT:
                if (x > 0)
                    next_x--;
                break;
            case Direction::RIGHT:
                if (x < width - 1)
                    next_x++;
                break;
            case Direction::NONE:
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
            return p->next[Direction::NONE];
        }
        p->visited = true;
        if (p->tunnel == Direction::NONE) {
            p->next[Direction::NONE] = p;
            return p;
        }
        vector<Point::Ptr> vec_p;
        Point::Ptr next_p = p;
        do {
            vec_p.emplace_back(next_p);
            next_p->visited = true;
            next_p = around_with_meteor_one_step(next_p, next_p->tunnel);
        } while (next_p->tunnel != Direction::NONE && !next_p->visited);
        if (next_p->tunnel == Direction::NONE ||
            around_with_meteor_one_step(next_p, next_p->tunnel) == next_p) {
            for (Point::Ptr &q : vec_p) {
                q->next[Direction::NONE] = next_p;
            }
            return next_p;
        } else if (next_p->next[Direction::NONE] != next_p) {
            for (Point::Ptr &q : vec_p) {
                q->next[Direction::NONE] = next_p->next[Direction::NONE];
            }
            return next_p->next[Direction::NONE];
        } else {
            // TODO loop
        }
    }

public:
    int width;
    int height;
    int vision;
    map<int, map<int, Point::Ptr>> maps;

    Team my_team;
    Team enemy_team;
};

#endif //AI_YANG_UTIL_H
