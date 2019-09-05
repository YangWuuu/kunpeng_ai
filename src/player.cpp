#include "player.h"

void Player::message_leg_start(cJSON *msg) {
    if (leg_info) {
        prev_leg_info = leg_info;
    }
    leg_info = make_shared<LegStartInfo>();
    parse_message_leg_start(msg);
    leg_info->construct_map();
    game = make_shared<Game>(leg_info);
    task_score = make_shared<TaskScore>();
    enemy_task_score = make_shared<TaskScore>();
}

string Player::message_leg_end(cJSON *msg) {
    cJSON *msg_data = cJSON_GetObjectItem(msg, "msg_data");
    cJSON *msg_teams = cJSON_GetObjectItem(msg_data, "teams");
    return string("round_id:") + to_string(round_info->round_id) + cJSON_PrintUnformatted(msg_teams);
}

string Player::message_round(cJSON *msg) {
    round_info = make_shared<RoundInfo>();
    parse_message_round(msg);
    //show_map();
    game->update_round_info(round_info);
    task_score->init_every_round(round_info->my_units);
    enemy_task_score->init_every_round(round_info->enemy_units);
    tree.root_node->executeTick();
    return pack_msg();
}
