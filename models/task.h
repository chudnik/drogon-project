#pragma once  // защита от повторного включения файла

#include <drogon/orm/DbClient.h>  // клиент БД
#include <drogon/orm/Mapper.h>    // Шаблонный класс для CRUD операций

#include <string>

namespace drogon_model {
struct Task {               // имя таблицы автоатически опредляется из имени структруры (Task -> tasks)
    int id{};               // INTEGER
    std::string title;      // TEXT
    bool completed{false};  // INTEGER (0/1)

    static constexpr auto kPrimaryKey = &Task::id;
};
}  // namespace drogon_model

namespace drogon {
namespace orm {
template <>
inline Mapper<drogon_model::Task> DbClient::mapper<drogon_model::Task>() {
    return Mapper<drogon_model::Task>(*this);  // создание экземпляра Mapper для Task передавая текущий объект DbClient
}
}  // namespace orm
}  // namespace drogon
