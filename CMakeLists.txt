cmake_minimum_required(VERSION 3.10)

project(lowLevelCommport)

add_subdirectory(Commport)

find_package(benchmark)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lbenchmark -std=c++11")

add_executable(${PROJECT_NAME} "main.cpp")


target_link_libraries(${PROJECT_NAME} CommPort)
target_link_libraries(${PROJECT_NAME} benchmark)
