#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Mapper.h>

#include "filters/api_key_filter.h"
#include "models/task.h"
#include "websockets/task_web_socket.h"

using namespace drogon;

class TaskController : public HttpController<TaskController> {
   public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(TaskController::getTasks, "/api/tasks", Get, "ApiKeyFilter");
    ADD_METHOD_TO(TaskController::addTask, "/api/tasks", Post, "ApiKeyFilter");
    ADD_METHOD_TO(TaskController::updateTask, "/api/tasks/{id}", Put, "ApiKeyFilter");
    ADD_METHOD_TO(TaskController::deleteTask, "/api/tasks/{id}", Delete, "ApiKeyFilter");
    METHOD_LIST_END

    void getTasks(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto db = app().getDbClient();
        drogon::orm::Mapper<drogon_model::Task> mapper(db);

        mapper.findAll(
            [callback = std::move(callback)](std::vector<drogon_model::Task> tasks) {
                Json::Value json(Json::arrayValue);
                for (auto& t : tasks) {
                    Json::Value item;
                    item["id"] = t.id;
                    item["title"] = t.title;
                    item["completed"] = t.completed;
                    json.append(item);
                }
                auto resp = HttpResponse::newHttpJsonResponse(json);
                callback(resp);
            },
            [callback](const drogon::orm::DrogonDbException& e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.base().what());
                callback(resp);
            });
    }

    void addTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto json = req->getJsonObject();
        if (!json || !(*json)["title"].isString()) {
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
        drogon::orm::Mapper<drogon_model::Task> mapper(db);

        mapper.insert(
            task,
            [callback](drogon_model::Task newTask) {
                BroadcastTasks();
                Json::Value item;
                item["id"] = newTask.id;
                item["title"] = newTask.title;
                item["completed"] = newTask.completed;
                auto resp = HttpResponse::newHttpJsonResponse(item);
                resp->setStatusCode(k201Created);
                callback(resp);
            },
            [callback](const drogon::orm::DrogonDbException& e) {
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
        drogon::orm::Mapper<drogon_model::Task> mapper(db);

        mapper.findOne(
            drogon::orm::Criteria("id", drogon::orm::CompareOperator::EQ, id),
            [this, db, json, id, callback = std::move(callback)](drogon_model::Task task) {
                if ((*json).isMember("title"))
                    task.title = (*json)["title"].asString();
                if ((*json).isMember("completed"))
                    task.completed = (*json)["completed"].asBool();

                drogon::orm::Mapper<drogon_model::Task> updateMapper(db);
                updateMapper.update(
                    task,
                    [callback](const size_t count) {
                        BroadcastTasks();
                        auto resp = HttpResponse::newHttpResponse();
                        resp->setStatusCode(count ? k200OK : k404NotFound);
                        callback(resp);
                    },
                    [callback](const drogon::orm::DrogonDbException& e) {
                        auto resp = HttpResponse::newHttpResponse();
                        resp->setStatusCode(k500InternalServerError);
                        resp->setBody(e.base().what());
                        callback(resp);
                    });
            },
            [callback](const drogon::orm::DrogonDbException& e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                callback(resp);
            });
    }

    void deleteTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
        auto db = app().getDbClient();
        drogon::orm::Mapper<drogon_model::Task> mapper(db);

        mapper.deleteBy(
            drogon::orm::Criteria("id", drogon::orm::CompareOperator::EQ, id),
            [callback](const size_t count) {
                BroadcastTasks();
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(count ? k204NoContent : k404NotFound);
                callback(resp);
            },
            [callback](const drogon::orm::DrogonDbException& e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.base().what());
                callback(resp);
            });
    }
};