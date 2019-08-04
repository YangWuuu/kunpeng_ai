project(BehaviorTree)

set(BEHAVIOR_TREE_LIBRARY ${PROJECT_NAME})
if (NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

list(APPEND BEHAVIOR_TREE_EXTERNAL_LIBRARIES
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS})


list(APPEND BT_SOURCE
        src/action_node.cpp
        src/basic_types.cpp
        src/behavior_tree.cpp
        src/blackboard.cpp
        src/bt_factory.cpp
        src/decorator_node.cpp
        src/condition_node.cpp
        src/control_node.cpp
        src/tree_node.cpp
        src/xml_parsing.cpp

        src/decorators/inverter_node.cpp
        src/decorators/repeat_node.cpp
        src/decorators/retry_node.cpp
        src/decorators/subtree_node.cpp
        src/decorators/timeout_node.cpp

        src/controls/fallback_node.cpp
        src/controls/parallel_node.cpp
        src/controls/reactive_sequence.cpp
        src/controls/reactive_fallback.cpp
        src/controls/sequence_node.cpp
        src/controls/sequence_star_node.cpp

        src/loggers/bt_cout_logger.cpp
        src/loggers/bt_file_logger.cpp
        src/loggers/bt_minitrace_logger.cpp
        src/private/tinyxml2.cpp

        3rdparty/minitrace/minitrace.cpp
        )

include_directories(include 3rdparty)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
add_library(${BEHAVIOR_TREE_LIBRARY} STATIC ${BT_SOURCE})

target_link_libraries(${BEHAVIOR_TREE_LIBRARY} PUBLIC ${BEHAVIOR_TREE_EXTERNAL_LIBRARIES})
