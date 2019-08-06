#include "game.h"

void GameState::updateRoundInfo(RoundInfo &ri) {
    round_id = ri.round_id;
    isEat = ri.mode == leg->my_team.force;
    my_units.clear();
    //TODO enemy predict
    enemy_units.clear();
    for (auto &u : ri.my_units) {
        auto &unit = *u.second;
        my_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
    }
    for (auto &u : ri.enemy_units) {
        auto &unit = *u.second;
        enemy_units[u.first] = Unit::gen(unit.id, unit.score, unit.sleep, unit.team, unit.loc);
    }
    for (int i = 0; i < leg->width; i++) {
        for (int j = 0; j < leg->height; j++) {
            background_score[i][j] = background_score_limit[i][j] / 30 * (round_id + 1 - visit_time[i][j]) ;
        }
    }
    for (auto &unit : my_units) {
        for (const auto &xy : getVisionGrids(unit.second->loc->x, unit.second->loc->y, leg->width, leg->height, leg->vision)) {
            power_score[xy.first][xy.second] = 0.0;
        }
    }
    for (auto &power : ri.powers) {
        background_score_limit[power.loc->x][power.loc->y] = power.point;
        power_score[power.loc->x][power.loc->y] = power.point;
    }
}

vector<Direction> GameState::getLegalActions(int agent_id) {
    Point::Ptr point = nullptr;
    if (my_units.count(agent_id) > 0) {
        point = my_units[agent_id]->loc;
    } else {
        point = enemy_units[agent_id]->loc;
    }
    vector<Direction> actions;
    actions.push_back(Direction::NONE);
    for (int i = 0; i < 4; i++){
        if (point->next[Direction(i)] != point) {
            actions.push_back(Direction(i));
        }
    }
    return actions;
}

shared_ptr<GameState> GameState::generateSuccessor(int agent_id, Direction action) {
    auto newGameState = make_shared<GameState>(shared_from_this());
    newGameState->round_id++;
    auto &unit = newGameState->my_units[agent_id];
    unit->loc = unit->loc->next[action];
    for (auto &xy : getVisionGrids(unit->loc->x, unit->loc->y, newGameState->leg->width, newGameState->leg->height, newGameState->leg->vision)) {
        newGameState->total_score += min(newGameState->background_score[xy.first][xy.second], newGameState->background_score_limit[xy.first][xy.second]);
        newGameState->background_score[xy.first][xy.second] = 0;
        newGameState->visit_time[xy.first][xy.second] = newGameState->round_id;
    }
    newGameState->total_score += newGameState->power_score[unit->loc->x][unit->loc->y];
    newGameState->power_score[unit->loc->x][unit->loc->y] = 0;
    if (!newGameState->isEat) {
        for (auto &eu : newGameState->enemy_units) {
            if (Point::distance(eu.second->loc, unit->loc) <= 1){  //TODO
                newGameState->total_score -= unit->score;
            }
        }
    }
    return newGameState;
}
