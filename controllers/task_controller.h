#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Mapper.h>

#include "filters/api_key_filter.h"
#include "models/task.h"
#include "websockets/task_websocket.h"

using namespace drogon;

class TaskController : public HttpController<TaskController> {  // обеспечивает регистрацию маршрутов через макросы
   public:
    METHOD_LIST_BEGIN  // макрос для определения маршрутов HTTP, разворачивается в статические структуры
        ADD_METHOD_TO(TaskController::getTasks, "api/tasks", Get, "ApiKeyFilter");  // фильтр приминяется перед вызовом метода
    ADD_METHOD_TO(TaskController::addTask, "api/tasks", Post, "ApiKeyFilter");
    ADD_METHOD_TO(TaskController::updateTask, "/api/tasks/{id}", Put, "ApiKeyFilter");
    ADD_METHOD_TO(TaskController::deleteTask, "/api/tasks/{id}", Delete, "ApiKeyFilter");
    METHOD_LIST_END

    // callback — функция, которую мы обязаны вызвать с готовым ответом
    void getTasks(const HttpRequestPtr& req, std::function<void(HttpRequestPtr)>&& callback) {
        auto db = app().getdbClient();                   // получаем клиент бд по умолчанию
        auto mapper = db->mapper<drogon_model::Task>();  // создаем маппер для модели Task из models/task.h
        mapper.findAll(                                  // ассинхронный запрос всех задач
            [callback = std::move(callback)](std::vector<drogon_model::Task> tasks) {
                Json::Value json(Json::arrayValue);
                for (auto& t : tasks) {
                    Json::Value item;
                    item["id"] = t.id;
                    item["title"] = t.title;
                    item["completed"] = t.completed;
                    json.append(item);
                }
                auto resp =
                    HttpResponse::newHttpJsonResponse(json);  // создаем ответ, автоматически устанавливая Content-Type: application/json

                callback(resp);  // отправляем ответ клиенту
            },
            [callback](const DrogonDbException& e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.base().what());
                callback(resp);
            });
    }

    void addTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto json = req->getJsonObject();
        if (!json || !(*json)["title"].isString()) {  // проверяем, что JSON объект существует и содержит строковое поле title
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            resp->setBody("Missing 'title'");
            callback(resp);
            return;
        }
        drogon_model::Task task;
        task.title = (*json)["title"].asString();
        task.completed = (*json).get("completed", false).asBool();
        auto db = app().getDbClient();
        auto mapper = db->mapper<drogon_model::Task>();
        mapper.insert(
            task,
            [callback](drogon_model::Task newTask) {
                broadcastTasks();  // оповещаем WS-клиентов
                Json::Value item;
                item["id"] = newTask.id;
                item["title"] = newTask.title;
                item["completed"] = newTask.completed;
                auto resp = HttpResponse::newHttpJsonResponse(item);
                resp->setStatusCode(k201Created);
                callback(resp);
            },
            [callback](const DrogonDbException& e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.base().what());
                callback(resp);
            });
    }

    void updateTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
        auto json = req->getJsonObject();
        if (!json) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        auto db = app().getDbClient();
        auto mapper = db->mapper<drogon_model::Task>();
        mapper.findOne(
            Criteria("id", CompareOperator::EQ, id),
            [this, db, json, id, callback = std::move(callback)](drogon_model::Task task) {
                if ((*json).isMember("title"))
                    task.title = (*json)["title"].asString();
                if ((*json).isMember("completed"))
                    task.completed = (*json)["completed"].asBool();
                auto mapper = db->mapper<drogon_model::Task>();
                mapper.update(
                    task,
                    [callback](const size_t count) {
                        broadcastTasks();
                        auto resp = HttpResponse::newHttpResponse();
                        resp->setStatusCode(count ? k200OK : k404NotFound);
                        callback(resp);
                    },
                    [callback](const DrogonDbException& e) { ... });
            },
            [callback](const DrogonDbException& e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                callback(resp);
            });
    }

    void deleteTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
        auto db = app().getDbClient();
        auto mapper = db->mapper<drogon_model::Task>();
        mapper.deleteBy(
            Criteria("id", CompareOperator::EQ, id),
            [callback](const size_t count) {
                broadcastTasks();
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(count ? k204NoContent : k404NotFound);
                callback(resp);
            },
            [callback](const DrogonDbException& e) { ... });
    }
};
