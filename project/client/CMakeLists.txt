add_library(client
    client.cc 
    client.h
    work.cc 
    work.h
    util.cc
    util.h
)

target_link_libraries(client -lssl -lcrypto)

# 设置头文件路径（如果有头文件的话）
target_include_directories(client
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

