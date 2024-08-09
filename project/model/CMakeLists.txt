add_library(model
    user.h 
    usermodel.h
    usermodel.cc
    friendmodel.h 
    friendmodel.cc
    groupmodel.h
    groupmodel.cc 
    groupuser.h 
    group.h
    filemodel.h
    filemodel.cc
    historyCasheManager.h
    historymodel.h 
    historymodel.cc
    privatechathistory.h
    groupchathistory.h
)

target_link_libraries(model)


# 设置头文件路径（如果有头文件的话）
target_include_directories(model
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

