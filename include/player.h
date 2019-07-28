#ifndef AI_YANG_PLAYER_H
#define AI_YANG_PLAYER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "cJSON.h"

#include "util.h"
#include "behaviortree_cpp/behavior_tree.h"
#include <behaviortree_cpp/bt_factory.h>
#include "customAction.h"
#include "customCondition.h"

#include "xml_tree.h"

using namespace std;

class Player {
public:
    explicit Player(int _team_id, string _team_name) : team_id(_team_id), team_name(move(_team_name)) {
        blackboard = BT::Blackboard::create();
        blackboard->set("leg", &leg);
        blackboard->set("ri", &ri);
        BT::BehaviorTreeFactory factory;

        factory.registerNodeType<IsEnemyAround>("IsEnemyAround");
        factory.registerNodeType<IsMyMode>("IsMyMode");
        factory.registerNodeType<IsPowerAround>("IsPowerAround");
        factory.registerNodeType<MoveToNearestPower>("MoveToNearestPower");
        factory.registerNodeType<RandomWalk>("RandomWalk");
        factory.registerNodeType<RunAway>("RunAway");

        tree = factory.createTreeFromText(xml_text, blackboard);
    }

    ~Player() = default;

    void message_leg_start(cJSON *msg);

    void message_leg_end(cJSON *msg);

    string message_round(cJSON *msg);

private:
    string pack_msg();

private:
    BT::Tree tree;
    BT::Blackboard::Ptr blackboard;
    LegStartInfo leg;
    RoundInfo ri;

    int team_id;
    string team_name;
};


#endif //AI_YANG_PLAYER_H
