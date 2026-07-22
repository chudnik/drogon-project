#pragma once

#include <drogon/plugins/Plugin.h>

class RequestTimerPlugin : public drogon::Plugin<RequestTimerPlugin> {
   public:
    void initAndStart(const Json::Value& config) override {
        LOG_INFO << "RequestTimerPlugin loaded";
    }

    void shutdown() override {}
};