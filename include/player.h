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
#include "xml_tree.h"
#include "game.h"
#include "customAction.h"
#include "assign_task.h"
#include "action_eat_enemy.h"
#include "action_eat_power.h"
#include "action_explore_map.h"
#include "action_run_away.h"
#include "make_decision.h"

using namespace std;

class Player {
public:
    explicit Player(int _team_id, string _team_name, bool _debug = false)
            : team_id(_team_id),
              team_name(move(_team_name)),
              debug(_debug) {
        leg_info = nullptr;
        prev_leg_info = nullptr;
        round_info = nullptr;
        game = nullptr;
        blackboard = BT::Blackboard::create();
        blackboard->set("info", this);
        BT::BehaviorTreeFactory factory;

        factory.registerNodeType<AssignTask>("AssignTask");
        factory.registerNodeType<CalculateShortestPath>("CalculateShortestPath");
        factory.registerNodeType<EatEnemy>("EatEnemy");
        factory.registerNodeType<EatPower>("EatPower");
        factory.registerNodeType<ExploreMap>("ExploreMap");
        factory.registerNodeType<MakeDecision>("MakeDecision");
        factory.registerNodeType<PredictEnemyNowLoc>("PredictEnemyNowLoc");
        factory.registerNodeType<RunAway>("RunAway");

        tree = factory.createTreeFromText(xml_text, blackboard);
        logger_cout.setTree(tree);
        //if (debug) {
        //    logger_file.setTree(tree, "bt_trace.fbl");
        //}
    }

    ~Player() = default;

    void message_leg_start(cJSON *msg);
    string message_leg_end(cJSON *msg);
    string message_round(cJSON *msg);

private:
    void parse_message_leg_start(cJSON *msg);
    void parse_message_round(cJSON *msg);
    string pack_msg();
    void show_map();

public:
    shared_ptr<LegStartInfo> leg_info;
    shared_ptr<LegStartInfo> prev_leg_info;
    shared_ptr<RoundInfo> round_info;
    shared_ptr<Game> game;
    shared_ptr<TaskScore> task_score;

    BT::Tree tree;
    BT::Blackboard::Ptr blackboard;

    int team_id;
    string team_name;

    bool debug;
    BT::StdCoutLogger logger_cout;
    BT::FileLogger logger_file;

public:

};
#endif //AI_YANG_PLAYER_H
