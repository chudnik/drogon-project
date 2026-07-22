#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Mapper.h>

#include <memory>

#include "models/task.h"

using namespace drogon;

class TaskWebSocket : public WebSocketController<TaskWebSocket> {  // обеспечивает автоматическую регистрацию и управление соединениями
   public:
    // conn - умный указатель на объект соединения
    // message - сообщение, rvalue ссылка, можно забрать без копирования
    // type - тип сообщения (текстовое, бинарное, ...)
    void handleNewMessage(const WebSocketConnectionPtr& conn, std::string&& message,
                          const WebSocketMessageType& type) override {  // обработчик входящего сообщения от клиента
        conn->send("pong");                                             // отправляем обратно подтверждение получения сообщения
    }

    void handleConnectionClosed(const WebSocketConnectionPtr& conn) override {  // обработчик закрытия соединения
        // автоматическое удаление
    }

    WS_PATH_LIST_BEGIN                                     // макрос для определения маршрутов WebSocket
        WS_PATH_ADD("/ws/tasks", "drogon::TaskWebSocket")  // добавление маршрута /ws/tasks и связывание с контроллером TaskWebSocket
        WS_PATH_LIST_END
};

inline void BroadcastTasks() {
    auto db = app().getDbClient();                   // получение клиента бд по умолчанию
    auto mapper = db->mapper<drogon_model::Task>();  // маппер для модели из models/task.h
    mapper.findAll(                                  // ассинхронно запрашиваем все записи из tasks
        [] std::vector<drogon_model::Task> tasks {   // первый callback принимает вектор задач
            Json::Value json(Json::arrayValue);      // создаем JSON значение типа массив
            for (auto& t : tasks) {                  // заполение JSON значения
                Json::Value item;
                item["id"] = t.id;
                item["title"] = t.title;
                item["completed"] = t.completed;
                json.append(item);
            }
            auto curl = app().getWebSocketController<TaskWebSocket>();  // получаем указатель на зарегестированный WebSocket контроллер
            if (curl) {  // если контроллер найден то отправляем JSON строку всем активным соеднинениям
                curl->getConnections().foreach ([&](const WebSocketConnectionPtr& conn) { conn->send(json.toStyledString()); });
            }
        },
        [](const DrogonDbException& e) {  // второй callback(ошибка), логируем через логгер Drogon
            LOG_ERROR << "DB error in broadcast: " << e.base().what();
        });
}