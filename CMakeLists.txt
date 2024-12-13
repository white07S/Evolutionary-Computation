cmake_minimum_required(VERSION 3.16)
project(evo_comp)

add_definitions(-std=c++17)

#add_library(build_settings INTERFACE)
#target_include_directories(build_settings INTERFACE src)

add_subdirectory(src)
add_subdirectory(lab1)
add_subdirectory(lab2)


# defines targets and sources
