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
    ENTER_CHATROOM,
    ADD_FRIEND,
    ADD_FRIEND_SUCCESS,
    ADD_FRIEND_REQUEST,
    UPDATE_FRIEND_LIST,
    DELETE_FRIEND,
    DELETED_FRIEND_SUCCESS,
    DELETE_FRIEND_SUCCESS,
    DELETE_FRIEND_FAIL,
    FRIEND_ONLINE,
    FRIEND_REQUEST_LIST,
    PRIVATE_CHAT,
    TEST,
};

#endif //PUBLIC_H
