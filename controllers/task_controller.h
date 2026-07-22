#pragma once

#include <drogon/HttpController.h>
#include <algorithm>

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

    void getTasks(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&& callback) {
        Json::Value json(Json::arrayValue);
        std::lock_guard<std::mutex> lock(task_store::mutex);
        for (const auto& task : task_store::tasks) json.append(task.toJson());
        callback(HttpResponse::newHttpJsonResponse(json));
    }

    void addTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        const auto json = req->getJsonObject();
        if (!json || !(*json)["title"].isString()) {
            respond(callback, k400BadRequest, "Missing 'title'");
            return;
        }
        Task task;
        {
            std::lock_guard<std::mutex> lock(task_store::mutex);
            task = {task_store::nextId++, (*json)["title"].asString(), (*json).get("completed", false).asBool()};
            task_store::tasks.push_back(task);
        }
        auto response = HttpResponse::newHttpJsonResponse(task.toJson());
        response->setStatusCode(k201Created);
        callback(response);
        BroadcastTasks();
    }

    void updateTask(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
        const auto json = req->getJsonObject();
        if (!json) {
            respond(callback, k400BadRequest);
            return;
        }
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(task_store::mutex);
            for (auto& task : task_store::tasks) {
                if (task.id != id) continue;
                if ((*json)["title"].isString()) task.title = (*json)["title"].asString();
                if ((*json)["completed"].isBool()) task.completed = (*json)["completed"].asBool();
                found = true;
                break;
            }
        }
        respond(callback, found ? k200OK : k404NotFound);
        if (found) BroadcastTasks();
    }

    void deleteTask(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(task_store::mutex);
            auto& tasks = task_store::tasks;
            const auto it = std::find_if(tasks.begin(), tasks.end(), [id](const Task& task) { return task.id == id; });
            if (it != tasks.end()) {
                tasks.erase(it);
                found = true;
            }
        }
        respond(callback, found ? k204NoContent : k404NotFound);
        if (found) BroadcastTasks();
    }

   private:
    static void respond(const std::function<void(const HttpResponsePtr&)>& callback,
                        HttpStatusCode status, const std::string& body = {}) {
        auto response = HttpResponse::newHttpResponse();
        response->setStatusCode(status);
        response->setBody(body);
        callback(response);
    }
};
