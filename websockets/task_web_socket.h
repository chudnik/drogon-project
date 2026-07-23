#pragma once

#include <drogon/WebSocketController.h>

#include <mutex>
#include <unordered_set>

#include "models/task.h"

using namespace drogon;

inline std::mutex connectionsMutex;
inline std::unordered_set<WebSocketConnectionPtr> connections;

class TaskWebSocket : public WebSocketController<TaskWebSocket> {
   public:
    void handleNewMessage(const WebSocketConnectionPtr& conn, std::string&& message, const WebSocketMessageType&) override {
        if (message == "ping")
            conn->send("pong");
    }
    void handleNewConnection(const HttpRequestPtr&, const WebSocketConnectionPtr& conn) override {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.insert(conn);
    }
    void handleConnectionClosed(const WebSocketConnectionPtr& conn) override {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.erase(conn);
    }
    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/tasks");
    WS_PATH_LIST_END
};

inline void BroadcastTasks() {
    Json::Value json(Json::arrayValue);
    {
        std::lock_guard<std::mutex> lock(task_store::mutex);
        for (const auto& task : task_store::tasks)
            json.append(task.toJson());
    }
    const auto message = json.toStyledString();
    std::lock_guard<std::mutex> lock(connectionsMutex);
    for (const auto& connection : connections)
        connection->send(message);
}
