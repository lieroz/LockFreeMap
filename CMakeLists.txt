cmake_minimum_required(VERSION 2.8.12)

project(LockFreeMap)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Core)
find_package(Qt5Test)

set(HEADERS
    "Atomic.h"
    "SimpleJobCoordinator.h"
    "QSBR.h" "QSBR.cpp"
    "MapTraits.h"
    "Util.h"
    "Leapfrog.h"
    "ConcurrentMap_Leapfrog.h")

add_executable(${PROJECT_NAME} "main.cpp" ${HEADERS})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Test pthread)
