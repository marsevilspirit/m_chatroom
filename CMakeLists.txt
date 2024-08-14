cmake_minimum_required(VERSION 3.10)

project(chatroom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 启用 AddressSanitizer
set(ENABLE_ASAN TRUE)

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

add_subdirectory(project/m_netlib)
add_subdirectory(project/server)
add_subdirectory(project/client)
add_subdirectory(project/model)
add_subdirectory(project/database)
add_subdirectory(project/threadpool)

add_executable(chatroom
    project/chatroom.cc
)

add_executable(chatclient
    project/chatclient.cc
)

target_link_libraries(chatroom server m_netlib model database threadpool)

target_link_libraries(chatclient client m_netlib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")

    message("Debug build type detected. Adding debug flags.")
    add_compile_options(-g -O0)  # 添加调试信息和关闭优化
endif()
