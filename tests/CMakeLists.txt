cmake_minimum_required(VERSION 2.8)
project(ai_test)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include ./include ../3rdparty/BehaviorTree/include ../3rdparty/cJSON ../3rdparty/log)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
aux_source_directory(. src_dir)
add_executable(${PROJECT_NAME} ${src_dir} ../src/player.cpp)
target_link_libraries(${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libBehaviorTree.a ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libcJSON.a ${CMAKE_CURRENT_SOURCE_DIR}/../lib/liblog.a)
add_test(Test MyTests)
enable_testing()
