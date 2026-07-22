#pragma once

#include <drogon/HttpMiddleware.h>

using namespace drogon;

class ApiKeyFilter : public HttpMiddleware<ApiKeyFilter> {
   public:
    void invoke(const HttpRequestPtr& req, MiddlewareNextCallback&& next, MiddlewareCallback&& callback) override {
        if (req->getHeader("X-API-Key") != "secret123") {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k401Unauthorized);
            response->setBody("Invalid API key");
            callback(response);
            return;
        }
        next(std::move(callback));
    }
};
