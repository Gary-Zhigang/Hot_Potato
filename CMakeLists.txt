cmake_minimum_required(VERSION 3.21)
project(hot_potato)

set(CMAKE_CXX_STANDARD 11)

add_executable(player player.cpp potato.hpp)
add_executable(ringmaster ringmaster.cpp potato.hpp)
