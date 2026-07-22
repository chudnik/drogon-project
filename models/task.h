#pragma once

#include <json/json.h>
#include <mutex>
#include <string>
#include <vector>

struct Task {
    int id{};
    std::string title;
    bool completed{};

    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["title"] = title;
        json["completed"] = completed;
        return json;
    }
};

namespace task_store {
inline std::mutex mutex;
inline std::vector<Task> tasks;
inline int nextId = 1;
}  // namespace task_store
