#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Mapper.h>

#include <memory>

#include "models/task.h"

using namespace drogon;

class TaskWebSocket : public WebSocketController<TaskWebSocket> {
   public:
    TaskWebSocket() = default;

    void handleNewMessage(const WebSocketConnectionPtr& conn, std::string&& message, const WebSocketMessageType& type) override {
        conn->send("pong");
    }

    void handleConnectionClosed(const WebSocketConnectionPtr& conn) override {}

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/tasks", "drogon::TaskWebSocket");
    WS_PATH_LIST_END
};

inline void BroadcastTasks() {
    auto db = app().getDbClient();
    drogon::orm::Mapper<drogon_model::Task> mapper(db);

    mapper.findAll(
        [](std::vector<drogon_model::Task> tasks) {
            Json::Value json(Json::arrayValue);
            for (auto& t : tasks) {
                Json::Value item;
                item["id"] = t.id;
                item["title"] = t.title;
                item["completed"] = t.completed;
                json.append(item);
            }
            auto& connections = TaskWebSocket::getConnections();
            connections.foreach ([&](const WebSocketConnectionPtr& conn) { conn->send(json.toStyledString()); });
        },
        [](const drogon::orm::DrogonDbException& e) { LOG_ERROR << "DB error in broadcast: " << e.base().what(); });
}