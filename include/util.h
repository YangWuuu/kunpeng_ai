#ifndef AI_YANG_UTIL_H
#define AI_YANG_UTIL_H

#include <map>
#include <memory>
#include <vector>
#include <random>

using namespace std;

static default_random_engine e(2019);

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

    static int distance(const Ptr &p1, const Ptr &p2){
        return abs(p1->x - p2->x) + abs(p1->y - p2->y);
    }

    explicit Point(int _x, int _y) : x(_x), y(_y), direction(Direction::NONE), meteor(false), wormhole(nullptr) {}

public:
    int x;
    int y;
    Direction direction;
    bool meteor;
    Ptr wormhole;
    map<Direction, Ptr> next;
};

class Unit {
public:
    using Ptr = std::shared_ptr<Unit>;

    static Unit::Ptr gen(int _id, int _score, int _sleep, int _team, Point::Ptr _loc) {
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
    Power(int _point, Point::Ptr _loc) : point(_point) {
        loc = move(_loc);
    }

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

    int width;
    int height;
    int vision;
    map<int, map<int, Point::Ptr>> maps;

    Team my_team;
    Team enemy_team;
};

#endif //AI_YANG_UTIL_H
