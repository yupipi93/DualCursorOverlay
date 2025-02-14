cmake_minimum_required(VERSION 3.10)
project(DualCursorOverlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(DualCursorOverlay dual_cursor.cpp)
target_link_libraries(DualCursorOverlay user32 hid) 