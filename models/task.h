#pragma once

#include <drogon/orm/Model.h>

#include <string>

namespace drogon_model {
class Task : public drogon::orm::Model<Task> {
   public:
    static constexpr auto kPrimaryKeyName = "id";

    int id{};
    std::string title;
    bool completed{false};

    static const std::string tableName;
};

const std::string Task::tableName = "tasks";
}  // namespace drogon_model