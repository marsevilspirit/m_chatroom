//Created by mars on 6/8/24

#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType{
    NEED_ENTER,     //需要进入
    REG_MSG,        //注册消息
    REG_SUCCESS,    //注册成功
    REG_FAIL,       //注册失败
    LOGIN_MSG,          //登录消息
    LOGIN_SUCCESS,
    LOGIN_REPEAT,
    LOGIN_FAIL,
    FRIEND_ONLINE,
    ENTER_CHATROOM,
    DISPLAY_ALLUSER_LIST,
    ADD_FRIEND,
    ADD_FRIEND_SUCCESS,
    ADD_FRIEND_REQUEST,
    TO_ADD_FRIEND_REQUEST,
    THE_OTHER_AGREE_FRIEND_REQUEST,
    UPDATE_FRIEND_LIST,
    DELETE_FRIEND,
    DELETED_FRIEND_SUCCESS,
    DELETE_FRIEND_SUCCESS,
    DELETE_FRIEND_FAIL,
    BLOCK_FRIEND,
    BLOCK_FRIEND_SUCCESS,
    UNBLOCK_FRIEND_SUCCESS,
    BLOCK_FRIEND_FAIL,
    UNBLOCK_FRIEND,
    UNBLOCK_FRIEND_FAIL,
    DISPLAY_FRIEND_LIST,
    FRIEND_REQUEST_LIST,
    BLOCK_FRIEND_LIST,
    CHECK_BLOCK,
    PRIVATE_CHAT,
    CREATE_GROUP,
    CREATE_GROUP_SUCCESS,
    CREATE_GROUP_FAIL,
    DISPLAY_ALLGROUP_LIST,
    REQUEST_GROUP,
    REQUEST_GROUP_SUCCESS,
    REQUEST_GROUP_FAIL,
    GROUP_REQUEST_LIST,
    DISPLAY_GROUP_REQUEST,
    ADD_GROUP,
    DISPLAY_GROUP_REQUEST_FAIL,
    ADD_GROUP_FAIL,
    ADD_GROUP_SUCCESS,
    QUIT_GROUP,
    QUIT_GROUP_FAIL,
    QUIT_GROUP_SUCCESS,
    DISPLAY_OWN_GROUP_LIST,
    DISPLAY_GROUP_MEMBER_LIST,
    DISPLAY_GROUP_MEMBER_FAIL,
    DISPLAY_GROUP_SET_MANAGER_MEMBER_LIST,
    DISPLAY_SET_MANAGER_MEMBER_FAIL,
    DISPLAY_SET_MANAGER_MEMBER_LIST,
    SET_MANAGER,
    SET_MANAGER_SUCCESS,
    SET_MANAGER_FAIL,
    CANCEL_MANAGER,
    CANCEL_MANAGER_FAIL,
    CANCEL_MANAGER_SUCCESS,
    KICK_SOMEONE,
    KICK_SOMEONE_FAIL,
    KICK_SOMEONE_SUCCESS,
    CHECK_GROUP_MEMBER,
    CHECK_IF_GROUP_MEMBER,
    GROUP_CHAT,
    MASTER_DELETE_GROUP,
    MASTER_DELETE_GROUP_FAIL,
    MASTER_DELETE_GROUP_SUCCESS,
    SEND_FILE,
    SEND_FILE_END,
    SEND_FILE_DATABASE,
    SEND_FILE_DATABASE_SUCCESS,
    SEND_FILE_SERVER,
    VIEW_FILE,
    RECEIVE_FILE,
    RECEIVE_FILE_FINISH,
    PRIVATE_CHAT_HISTORY,
    DISPLAY_PRIVATE_HISTORY,
    GROUP_CHAT_HISTORY,
    DISPLAY_GROUP_HISTORY,
    CLIENT_LONGTIME_EXIT,
    RECEIVE_FILE_FAIL,
};

#endif //PUBLIC_H
