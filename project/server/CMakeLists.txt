add_library(server
    server.cc
    server.h
    service.cc
    service.h
)

target_link_libraries(server)

# 设置头文件路径（如果有头文件的话）
target_include_directories(server
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
