#pragma once

#include <drogon/drogon_callbacks.h>
#include <drogon/plugins/Plugin.h>

class RequestTimerPlugin : public drogon::Plugin<RequestTimerPlugin> {
   public:
    void initAndStart(const Json::Value& config) override {
        drogon::app().registerBeginningAdvice([]() {
            auto* timer = new std::chrono::steady_clock::time_point;
            *timer = std::chrono::steady_clock::now();
            drogon::app().getCurrentThreadContext()->setThreadLocalData("request_start", timer);
        });
        drogon::app().registerEndingAdvice([]() {
            auto* timer = static_cast<std::chrono::steady_clock::time_point*>(
                drogon::app().getCurrentThreadContext()->getThreadLocalData("request_start"));
            if (timer) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - *timer).count();
                LOG_INFO << "Request processed in " << duration << " ms";
                delete timer;
            }
        });
    }

    void shutdown() override {}
};