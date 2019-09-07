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
#include "assign_task.h"
#include "predict_enemy.h"
#include "action_eat_enemy.h"
#include "action_eat_power.h"
#include "action_explore_map.h"
#include "action_run_away.h"
#include "action_search_enemy.h"
#include "action_avoid_enemy.h"
#include "action_out_vision.h"
#include "action_remove_invalid.h"
#include "make_decision.h"

using namespace std;

class Player {
public:
    explicit Player(int _team_id)
            : team_id(_team_id) {
        leg_info = nullptr;
        prev_leg_info = nullptr;
        round_info = nullptr;
        game = nullptr;
        blackboard = BT::Blackboard::create();
        blackboard->set("info", this);
        BT::BehaviorTreeFactory factory;

        factory.registerNodeType<AssignTask>("AssignTask");
        factory.registerNodeType<EatEnemy>("EatEnemy");
        factory.registerNodeType<EatPower>("EatPower");
        factory.registerNodeType<ExploreMap>("ExploreMap");
        factory.registerNodeType<MakeDecision>("MakeDecision");
        factory.registerNodeType<PredictEnemyNowLoc>("PredictEnemyNowLoc");
        factory.registerNodeType<RunAway>("RunAway");
        factory.registerNodeType<SearchEnemy>("SearchEnemy");
        factory.registerNodeType<AvoidEnemy>("AvoidEnemy");
        factory.registerNodeType<OutVision>("OutVision");
        factory.registerNodeType<RemoveInvalid>("RemoveInvalid");

        tree = factory.createTreeFromText(xml_text, blackboard);
        logger_cout.setTree(tree);
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
    shared_ptr<TaskScore> enemy_task_score;

    BT::Tree tree;
    BT::Blackboard::Ptr blackboard;

    int team_id;

    BT::StdCoutLogger logger_cout;

public:

};
#endif //AI_YANG_PLAYER_H
