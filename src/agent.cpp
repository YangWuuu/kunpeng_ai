#include "agent.h"

#include <algorithm>
#include <functional>

#include "log.h"

map<int, Direction> Agent::getActions(const shared_ptr<GameState> &game_state) {
    map<int, Direction> ret;
    auto new_game_state = game_state;
    for (auto &mu : game_state->my_units) {
        Direction direction = getAction(mu.first, new_game_state, 5);
        ret[mu.first] = direction;
        new_game_state = game_state->generateSuccessor(mu.first, direction);
        log_info("id: %d : %d  score: %f", mu.first, direction, new_game_state->total_score);
    }
    return ret;
}

Direction Agent::getAction(int agent_id, const shared_ptr<GameState> &game_state, int depth) {
    function<double(int, const shared_ptr<GameState> &, int)> minimax;
    minimax = [&](int agent_id, const shared_ptr<GameState> &game_state, int depth) -> double {
        if (game_state->isEnd() || depth == 0) {
            return game_state->total_score;
        }
        depth--;
        vector<double> score;
        for (Direction direction : game_state->getLegalActions(agent_id)) {
            score.push_back(minimax(agent_id, game_state->generateSuccessor(agent_id, direction), depth));
        }
        return *max_element(score.begin(), score.end());
    };
    double best_score = numeric_limits<double>::min();
    Direction best_direction = Direction::NONE;
    for (Direction direction : game_state->getLegalActions(agent_id)) {
        double score = minimax(agent_id, game_state->generateSuccessor(agent_id, direction), depth);
        if (score > best_score) {
            best_score = score;
            best_direction = direction;
        }
    }
    return best_direction;
}
