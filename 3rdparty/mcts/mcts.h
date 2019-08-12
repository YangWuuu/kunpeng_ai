/*
A very simple C++11 Templated MCTS (Monte Carlo Tree Search) implementation with examples for openFrameworks. 

MCTS Code Based on the Java (Simon Lucas - University of Essex) and Python (Peter Cowling, Ed Powley, Daniel Whitehouse - University of York) impelementations at http://mcts.ai/code/index.html
*/

#pragma once

#include <cfloat>
#include "TreeNodeT.h"
#include "LoopTimer.h"

namespace mcts {

    // State must comply with State Interface (see IState.h)
    // Action can be anything (which your State class knows how to handle)
    template<class State, typename Action>
    class UCT {
        typedef TreeNodeT<State, Action> TreeNode;
        typedef std::shared_ptr<TreeNodeT<State, Action>> Ptr;

    private:
        LoopTimer timer;
        int iterations;

    public:
        double uct_k;                    // k value in UCT function. default = sqrt(2)
        unsigned int max_iterations;    // do a maximum of this many iterations (0 to run till end)
        unsigned int max_millis;        // run for a maximum of this many milliseconds (0 to run till end)
        unsigned int simulation_depth;    // how many ticks (frames) to run simulation for

        //--------------------------------------------------------------
        UCT() :
                iterations(0),
                uct_k(sqrt(2)),
                max_iterations(100000),
                max_millis(500),
                simulation_depth(40) {}

        //--------------------------------------------------------------
        const LoopTimer &get_timer() const {
            return timer;
        }

        int get_iterations() const {
            return iterations;
        }

        //--------------------------------------------------------------
        // get best (immediate) child for given TreeNode based on uct score
        Ptr get_best_uct_child(Ptr node) const {
            // sanity check
            if (!node->is_fully_expanded()) return nullptr;

            double best_utc_score = -std::numeric_limits<double>::max();
            Ptr best_node = nullptr;

            // iterate all immediate children and find best UTC score
            int num_children = node->get_num_children();
            for (int i = 0; i < num_children; i++) {
                Ptr child = node->get_child(i);
                double uct_exploitation = (double) child->get_value() / (child->get_num_visits() + DBL_EPSILON);
                double uct_exploration = sqrt(
                        log((double) node->get_num_visits() + 1) / (child->get_num_visits() + DBL_EPSILON));
                double uct_score = uct_exploitation + uct_k * uct_exploration;

                if (uct_score > best_utc_score) {
                    best_utc_score = uct_score;
                    best_node = child;
                }
            }
            return best_node;
        }

        //--------------------------------------------------------------
        Ptr get_most_visited_child(Ptr node) const {
            int most_visits = -1;
            Ptr best_node = nullptr;
            // iterate all immediate children and find most visited
            int num_children = node->get_num_children();
            for (int i = 0; i < num_children; i++) {
                Ptr child = node->get_child(i);
                if (child->get_num_visits() > most_visits) {
                    most_visits = child->get_num_visits();
                    best_node = child;
                }
            }
            return best_node;
        }
        //--------------------------------------------------------------
        Ptr run(const State &current_state) {
            // initialize timer
            timer.init();
            // initialize root TreeNode with current state
            auto root_node = make_shared<TreeNode>(current_state);
            // iterate
            iterations = 0;
            while (true) {
                // indicate start of loop
                timer.loop_start();

                // 1. SELECT. Start at root, dig down into tree using UCT on all fully expanded nodes
                Ptr node = root_node;
                while (!node->is_terminal() && node->is_fully_expanded()) {
                    node = get_best_uct_child(node);
                }

                // 2. EXPAND by adding a single child (if not terminal or not fully expanded)
                if (!node->is_fully_expanded() && !node->is_terminal()) node = node->expand();

                State state(node->get_state());

                // 3. SIMULATE (if not terminal)
                if (!node->is_terminal()) {
                    Action action;
                    for (int t = 0; t < simulation_depth; t++) {
                        if (state.is_terminal()) break;

                        if (state.get_random_action(action))
                            state.apply_action(action);
                        else
                            break;
                    }
                }

                // get rewards vector for all agents
                const std::vector<double> rewards = state.evaluate();

                // 4. BACK PROPAGATION
                while (node) {
                    node->update(rewards);
                    node = node->get_parent();
                }

                // indicate end of loop for timer
                timer.loop_end();

                // exit loop if current total run duration (since init) exceeds max_millis
                if (max_millis > 0 && timer.check_duration(max_millis)) break;

                // exit loop if current iterations exceeds max_iterations
                if (max_iterations > 0 && iterations > max_iterations) break;
                iterations++;
            }
            return root_node;
        }
    };
}

