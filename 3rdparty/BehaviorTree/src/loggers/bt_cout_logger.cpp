#include "behaviortree_cpp/loggers/bt_cout_logger.h"

#include "log.h"

namespace BT {
    std::atomic<bool> StdCoutLogger::ref_count(false);

    StdCoutLogger::StdCoutLogger() : StatusChangeLogger() {
    }

    StdCoutLogger::~StdCoutLogger() {
        ref_count.store(false);
    }

    void StdCoutLogger::setTree(const BT::Tree& tree) {
        StatusChangeLogger::setRootNode(tree.root_node);
        bool expected = false;
        if (!ref_count.compare_exchange_strong(expected, true)) {
            throw LogicError("Only one instance of StdCoutLogger shall be created");
        }
    }

    void StdCoutLogger::callback(Duration timestamp, const TreeNode &node, NodeStatus prev_status,
                                 NodeStatus status) {
        using namespace std::chrono;

        constexpr const char *whitespaces = "                         ";
        constexpr const size_t ws_count = 25;

        double since_epoch = duration<double>(timestamp).count();
        log_info_w("%s%s %s -> %s",
               node.name().c_str(),
               &whitespaces[std::min(ws_count, node.name().size())],
               toStr(prev_status, false).c_str(),
               toStr(status, false).c_str());
//        std::cout << std::endl;
    }

    void StdCoutLogger::flush() {
//        std::cout << std::flush;
        ref_count = false;
    }

}   // end namespace
