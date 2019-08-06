#ifndef AI_YANG_AGENT_H
#define AI_YANG_AGENT_H

#include <map>

#include "util.h"
#include "game.h"

using namespace std;

class Agent {
public:
    static map<int, Direction> getActions(const shared_ptr<GameState>& gameState);
    static Direction getAction(int agent_id, const shared_ptr<GameState>& gameState, int depth);
public:
};

#endif //AI_YANG_AGENT_H
