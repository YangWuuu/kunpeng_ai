#ifndef AI_YANG_AGENT_H
#define AI_YANG_AGENT_H

#include <map>

#include "util.h"
#include "game.h"

using namespace std;

class Agent {
public:
    map<int, Direction> getActions(GameState *gameState);
    Direction getAction(int agent_id, GameState *gameState);
public:
};

#endif //AI_YANG_AGENT_H
