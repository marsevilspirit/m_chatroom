//Created by mars on 7/8/24

#ifndef  JSON_CODEC_H
#define  JSON_CODEC_H

#include "json.hpp"
#include "m_netlib/Net/TcpConnection.h"
#include "m_netlib/Base/Timestamp.h"
#include "m_netlib/Log/mars_logger.h"
#include "public.h"

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using json = nlohmann::json;

struct FileTransferState {
    std::shared_ptr<std::ofstream> outfile;
    size_t fileSize = 0;
};

class JsonCodec {
public:
    using JsonMessageCallback = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

    explicit JsonCodec(JsonMessageCallback cb)
        : m_messageCallback(std::move(cb)) {}

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        while (buf->readableBytes() > 0) {
            if (m_fileTransferStates.find(conn) == m_fileTransferStates.end()) {
                // 不在文件传输模式
                const char* crlf = buf->findCRLF();
                if (crlf) {
                    std::string jsonString(buf->peek(), crlf);
                    buf->retrieveUntil(crlf + 2); // +2 to remove "\r\n"

                    try {
                        json js = json::parse(jsonString);
                        int msgid = js["msgid"].get<int>();

                        if (msgid == SEND_FILE || msgid == SEND_FILE_SERVER) {
                            std::string filename = js["filename"].get<std::string>();
                            size_t fileSize = js["filesize"].get<size_t>();
                            
                            std::string fullpath;
                            
                            if(msgid == SEND_FILE)
                            {
                                fullpath = "./received_files/" + filename;
                            } else {
                                fullpath = "./" + filename;
                            }

                            auto outfile = std::make_shared<std::ofstream>(fullpath, std::ios::binary);
                            if (!outfile->is_open()) {
                                LogError("Failed to open file for writing: {}", fullpath);
                                conn->shutdown();
                                return;
                            }

                            // 将文件传输状态存储在 map 中
                            m_fileTransferStates[conn] = {outfile, fileSize};
                        } else if (msgid == SEND_FILE_END) {
                            auto it = m_fileTransferStates.find(conn);
                            if (it != m_fileTransferStates.end()) {
                                it->second.outfile->close();
                                m_fileTransferStates.erase(it); // 清除状态
                            }
                        } else {
                            m_messageCallback(conn, js, time);
                        }
                    } catch (const json::parse_error& e) {
                        LogError("json parse error: {}", e.what());
                        conn->shutdown();
                    }
                } else {
                    break; // not enough data to form a complete message
                }
            } else {
                // 正在接收文件数据
                auto& state = m_fileTransferStates[conn];
                size_t readable = std::min(buf->readableBytes(), state.fileSize);
                state.outfile->write(buf->peek(), readable);
                buf->retrieve(readable);

                state.fileSize -= readable;
                if (state.fileSize == 0) {
                    state.outfile->close();
                    m_fileTransferStates.erase(conn); // 清除状态
                }
            }
        }
    }

    void send(const TcpConnectionPtr& conn, const json& message) {
        std::string jsonString = message.dump();
        jsonString.append("\r\n"); // Append CRLF to indicate end of message
        conn->send(jsonString);
    }

private:
    JsonMessageCallback m_messageCallback;
    std::unordered_map<TcpConnectionPtr, FileTransferState> m_fileTransferStates;
};

#endif //JSON_CODEC_H
