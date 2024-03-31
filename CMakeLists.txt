cmake_minimum_required(VERSION 3.25)
project(babayan)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost COMPONENTS 
    program_options
    system
    filesystem
    regex
    coroutine
    thread
    timer
    unit_test_framework
    REQUIRED
)
include_directories(${Boost_INCLUDE_DIR})

include_directories(inc)

add_executable(${PROJECT_NAME} src/${PROJECT_NAME}.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

add_executable(keeper_test test/keeper_test.cc)
target_link_libraries(keeper_test ${Boost_LIBRARIES})

enable_testing()
add_test(keeper_test keeper_test)

