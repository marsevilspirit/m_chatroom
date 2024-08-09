add_library(database
    database.cc 
    database.h
    redis.h
    redis.cc
)

target_link_libraries(database -lmysqlclient -lhiredis)

# 设置头文件路径（如果有头文件的话）
target_include_directories(database
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
