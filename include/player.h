#ifndef AI_YANG_PLAYER_H
#define AI_YANG_PLAYER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "util.h"
#include "cJSON.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_file_logger.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "customAction.h"
#include "customCondition.h"

#include "xml_tree.h"

using namespace std;

class Player {
public:
    explicit Player(int _team_id, string _team_name, bool _debug = false)
            : team_id(_team_id),
              team_name(move(_team_name)),
              debug(_debug) {
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
        if (debug) {
            printTreeRecursively(tree.root_node);
            logger_cout.setTree(tree);
            logger_file.setTree(tree, "bt_trace.fbl");
        }
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

    bool debug;
    BT::StdCoutLogger logger_cout;
    BT::FileLogger logger_file;
};


#endif //AI_YANG_PLAYER_H
