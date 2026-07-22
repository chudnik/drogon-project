cmake_minimum_required(VERSION 3.16)
project(DrogonTaskManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon REQUIRED)

add_executable(drogontaskmanager 
    main.cc
)

target_include_directories(drogontaskmanager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(drogontaskmanager PRIVATE Drogon::Drogon)