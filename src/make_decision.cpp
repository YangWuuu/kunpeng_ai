#include "make_decision.h"

#include "player.h"

BT::NodeStatus MakeDecision::tick() {
    auto info = config().blackboard->get<Player *>("info");

    int score_num = info->task_score->score_num;
    vector<double> all_good_score = vector<double>(score_num, 0.0);
    for (auto &gc : info->task_score->good_score) {
        for (int i = 0; i < score_num; i++) {
            all_good_score[i] += gc.second[i];
        }
    }
    double max_good_score = *max_element(all_good_score.begin(), all_good_score.end());
    vector<int> good_score_id;
    for (int i = 0; i < score_num; i++) {
        if (equal_double(all_good_score[i], max_good_score))
            good_score_id.push_back(i);
    }
    int id = good_score_id[uniform_int_distribution<int>(0, good_score_id.size() - 1)(e)];
    for (auto &md : info->task_score->get_map_direction(id)) {
        auto &mu = info->round_info->my_units[md.first];
        if (mu) {
            mu->direction = md.second;
        }
    }

    return BT::NodeStatus::SUCCESS;
}
