/*
A TreeNode in the decision tree.
I tried to keep this independent of UCT/MCTS.
Only contains information / methods related to State, Action, Parent, Children etc. 

*/

#pragma once

#include <memory>
#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>

namespace mcts {

    template<class State, typename Action>
    class TreeNodeT : public enable_shared_from_this<TreeNodeT<State, Action>> {
        typedef std::shared_ptr<TreeNodeT<State, Action> > Ptr;
        typedef std::weak_ptr<TreeNodeT<State, Action> > WeakPtr;

    public:
        //--------------------------------------------------------------
        explicit TreeNodeT(const State &state, Ptr parent = nullptr) :
                state(state),
                action(),
                parent(parent),
                agent_id(state.agent_id()),
                num_visits(0),
                value(0),
                depth(parent ? parent->depth + 1 : 0) {
        }

        //--------------------------------------------------------------
        // expand by adding a single child
        Ptr expand() {
            // sanity check that we're not already fully expanded
            if (is_fully_expanded()) return nullptr;

            // sanity check that we don't have more children than we do actions
            //assert(children.size() < actions.size()) ;

            // if this is the first expansion and we haven't yet got all of the possible actions
            if (actions.empty()) {
                // retrieve list of actions from the state
                state.get_actions(actions);

                // randomize the order
                std::random_shuffle(actions.begin(), actions.end());
            }

            // add the next action in queue as a child
            return add_child_with_action(actions[children.size()]);
        }

        //--------------------------------------------------------------
        void update(const std::vector<double> &rewards) {
            this->value += rewards[agent_id];
            num_visits++;
        }

        //--------------------------------------------------------------
        // GETTERS
        // state of the TreeNode
        const State &get_state() const { return state; }

        // the action that led to this state
        const Action &get_action() const { return action; }

        // all children have been expanded and simulated
        bool is_fully_expanded() const { return !children.empty() && children.size() == actions.size(); }

        // does this TreeNode end the search (i.e. the game)
        bool is_terminal() const { return state.is_terminal(); }

        // number of times the TreeNode has been visited
        int get_num_visits() const { return num_visits; }

        // accumulated value (wins)
        double get_value() const { return value; }

        // how deep the TreeNode is in the tree
        int get_depth() const { return depth; }

        // number of children the TreeNode has
        int get_num_children() const { return children.size(); }

        // get the i'th child
        Ptr get_child(int i) const { return children[i]; }

        // get parent
        Ptr get_parent() const { return parent.lock(); }

    private:
        State state;            // the state of this TreeNode
        int agent_id;            // agent who made the decision
        Action action;            // the action which led to the state of this TreeNode
        WeakPtr parent;        // parent of this TreeNode

        int num_visits;            // number of times TreeNode has been visited
        double value;            // value of this TreeNode
        int depth;

        std::vector<Ptr> children;    // all current children
        std::vector<Action> actions;            // possible actions from this state

        //--------------------------------------------------------------
        // create a clone of the current state, apply action, and add as child
        Ptr add_child_with_action(const Action &new_action) {
            // create a new TreeNode with the same state (will get cloned) as this TreeNode
            auto child_node = make_shared<TreeNodeT>(state, this->shared_from_this());

            // set the action of the child to be the new action
            child_node->action = new_action;

            // apply the new action to the state of the child TreeNode
            child_node->state.apply_action(new_action);

            // add to children
            children.emplace_back(child_node);

            return child_node;
        }
    };
}
