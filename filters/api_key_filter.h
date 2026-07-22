#pragma once

#include <drogon/HttpFilter.h>

using namespace drogon;

class ApiKeyFilter : public HttpFilter<ApiKeyFilter> {  // позволяет Drogon автоматически регистрировать фильтр по имени класса
   public:
    // req - умный указатель на объект запроса
    // fcb - функция обратного вызова, которую нужно вызвать для прерывания цепочки и отправки ответа
    // fccb - функция обратного вызова для продолжения цепочки фильтров
    void doFilter(const HttpRequestPtr& req, FilterCallback&& fcb, FilterChainCallback&& fccb) override {
        if (req->getHeader("X-API-Key") == "secret123") {  // получем значение HTTP заголовка и сравниваем с ожидаемым
            fccb();                                        // если верно, то продолжаем цепочку фильтров
        } else {
            auto resp = HttpResponse::newHttpResponse();  // создаем новый объект HTTP ответа
            resp->setStatusCode(k401Unauthorized);        // установка HTTP статуса 401
            resp->setBody("Invalid API Key");             // установка тела ответа
            fcb(resp);  // передаем сформированный ответ и прерывем цепочку фильтров, сразу отпраляя ответ клиенту
        }
    }
};