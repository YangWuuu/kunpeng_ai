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
            background_score[i][j] = background_score_limit[i][j] / 10 * (round_id - visit_time[i][j]) ;
        }
    }
    for (auto &power : ri.powers) {
        background_score_limit[power.loc->x][power.loc->y] = power.point;
        power_score[power.loc->x][power.loc->y] = power.point;
    }
    for (auto &unit : my_units) {
        for (const auto &xy : getVisionGrids(unit.second->loc->x, unit.second->loc->y, leg->width, leg->height, leg->vision)) {
            //TODO clean eated power
        }
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

GameState* GameState::generateSuccessor(int agent_id, Direction action) {
    auto newGameState = new GameState(this);
    auto &unit = newGameState->my_units[agent_id];
    unit->loc = unit->loc->next[action];
    for (auto &xy : getVisionGrids(unit->loc->x, unit->loc->y, leg->width, leg->height, leg->vision)) {
        total_score += min(background_score[xy.first][xy.second], background_score_limit[xy.first][xy.second]);
        background_score[xy.first][xy.second] = 0;
        visit_time[xy.first][xy.second] = round_id;
    }
    return newGameState;
}
