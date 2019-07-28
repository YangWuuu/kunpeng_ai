#include "behaviortree_cpp/decorators/subtree_node.h"


BT::DecoratorSubtreeNode::DecoratorSubtreeNode(const std::string &name, const NodeConfiguration& config) :
    DecoratorNode(name, config )
{
    setRegistrationID("SubTree");
}

BT::NodeStatus BT::DecoratorSubtreeNode::tick()
{
    NodeStatus prev_status = status();
    if (prev_status == NodeStatus::IDLE)
    {
        setStatus(NodeStatus::RUNNING);
    }
    return child_node_->executeTick();
}

