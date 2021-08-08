cmake_minimum_required(VERSION 3.4.3)
project(liveterm VERSION 1.0)

# include(CheckIncludeFile)
# CHECK_INCLUDE_FILE("termios.h" TERMIOS)
# if(NOT VECTOR)
# 	message(FATAL_ERROR "termios not found")
# endif()

find_package(Threads)

add_library(liveterm liveterm.cpp)
target_include_directories(liveterm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(liveterm Threads::Threads)

add_executable(termdemo example.cpp)
target_link_libraries(termdemo liveterm)
