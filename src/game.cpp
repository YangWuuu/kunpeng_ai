#include "game.h"

#include <algorithm>

void Game::update_round_info(shared_ptr<RoundInfo> &round_info) {
    round_id = round_info->round_id;
    is_eat = round_info->mode == leg_info->my_team.force;
    my_units.clear();
    //TODO enemy predict
    enemy_units.clear();
    for (auto &u : round_info->my_units) {
        auto &unit = *u.second;
        my_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
    }
    for (auto &u : round_info->enemy_units) {
        auto &unit = *u.second;
        enemy_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
    }
    for (int i = 0; i < leg_info->width; i++) {
        for (int j = 0; j < leg_info->height; j++) {
            background_score[i][j] = background_score_limit[i][j] / 60 * (round_id + 1 - visit_time[i][j]);
        }
    }
    for (auto &unit : my_units) {
        for (const auto &xy : getVisionGrids(unit.second->loc->x, unit.second->loc->y, leg_info->width, leg_info->height,
                                             leg_info->vision)) {
            power_score[xy.first][xy.second] = 0.0;
        }
    }
    for (auto &power : round_info->powers) {
        background_score_limit[power.loc->x][power.loc->y] = (double)power.point / (leg_info->vision * leg_info->vision);
        power_score[power.loc->x][power.loc->y] = power.point;
    }
}

double Game::getAreaScore(Point::Ptr &center, int area) {
    double area_score = 0.0;
    for (auto &xy : getVisionGrids(center->x, center->y, leg_info->width, leg_info->height, area)) {
        area_score += min(background_score[xy.first][xy.second], background_score_limit[xy.first][xy.second]);
    }
    area_score += power_score[center->x][center->y];
    for (auto &eu : enemy_units) {
        if (Point::distance(eu.second->loc, center) <= 2) {
            if (is_eat) {
//                if (Point::distance(eu.second->loc, center) == 1)
                area_score += eu.second->score / 20.0;
            } else {
                area_score -= 5.0;
            }
        }
    }
    for (auto &mu : my_units) {
        if (Point::distance(mu.second->loc, center) <= 0) {
            area_score -= 20;
        }
    }
    return area_score;
}

Point::Ptr Game::getBestGoal(Point::Ptr &start) {
    Point::Ptr ret = nullptr;
    double best_single_score = -numeric_limits<double>::max();
    for (auto &mm : leg_info->maps) {
        for (auto &m : mm.second) {
            double single_score = getAreaScore(m.second, leg_info->vision) / (1 + leg_info->path.get_cost(start, m.second));
            if (single_score > best_single_score) {
                best_single_score = single_score;
                ret = m.second;
            }
        }
    }
    return ret;
}

DIRECTION Game::checkTunnel(int agent_id, DIRECTION action) {
    DIRECTION ret = action;
    if (!is_eat) {
        bool in_danger = false;
        Point::Ptr next_loc = my_units[agent_id]->loc->next[action];
        for (auto &eu : enemy_units) {
            if (leg_info->path.get_cost(next_loc, eu.second->loc) <= 2) {
                in_danger = true;
                break;
            }
        }
        if (!in_danger) {
            if (!inVision(my_units[agent_id]->loc->next[ret]) && !my_units[agent_id]->loc->next[ret]->wormhole) {
                //maybe in danger
                vector<DIRECTION> choice_d;
                for (int i = 0; i < 5; i++) {
                    if (inVision(my_units[agent_id]->loc->next[DIRECTION(i)])) {
                        choice_d.push_back(DIRECTION(i));
                    }
                }
                if (!choice_d.empty()) {
                    ret = choice_d[uniform_int_distribution<int>(0, choice_d.size() - 1)(e)];
                }
            }
        }
    }
    return ret;
}

DIRECTION Game::checkRunAway(int agent_id, DIRECTION action) {
    DIRECTION ret = checkTunnel(agent_id, action);
    if (!is_eat) {
        bool in_danger = false;
        Point::Ptr next_loc = my_units[agent_id]->loc->next[action];
        for (auto &eu : enemy_units) {
            if (leg_info->path.get_cost(next_loc, eu.second->loc) <= 1) {
                in_danger = true;
                break;
            }
        }
        if (in_danger) {
            auto move_step = [&](DIRECTION direction) -> int {
                int min_distance = 0xffff;
                for (auto &eu : enemy_units) {
                    int d = Point::distance(my_units[agent_id]->loc->next[direction], eu.second->loc);
                    if (d < min_distance) {
                        min_distance = d;
                    }
                }
                return min_distance;
            };
            vector<DIRECTION> choice_d;
            for (int i = 0; i < 5; i++) {
                if (move_step(DIRECTION(i)) > 1) {
                    choice_d.push_back(DIRECTION(i));
                }
            }
            if (!choice_d.empty() && find(choice_d.begin(), choice_d.end(), ret) == choice_d.end()) {
                ret = choice_d[uniform_int_distribution<int>(0, choice_d.size() - 1)(e)];
            }
        }
    }
    return ret;
}

bool Game::inVision(Point::Ptr &loc) {
    bool ret = false;
    for (auto &mu : my_units) {
        if (Point::distance(mu.second->loc, loc) <= leg_info->vision) {
            ret = true;
            break;
        }
    }
    return ret;
}
