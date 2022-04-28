cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(state_machine LANGUAGES CXX)

add_library(state_machine_lib STATIC
  state_machine.h
  state_machine.cpp
)

target_include_directories(state_machine_lib
  PUBLIC
    .
)

add_executable(state_machine_app state_machine_app.cpp)

target_link_libraries(state_machine_app
  PUBLIC
    state_machine_lib
)
