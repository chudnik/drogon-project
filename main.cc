#include <drogon/drogon.h>

#include "controllers/task_controller.h"
#include "filters/api_key_filter.h"
#include "plugins/request_timer_plugin.h"
#include "websockets/task_web_socket.h"

using namespace drogon;

int main() {
    (void)ApiKeyFilter::classTypeName();
    (void)RequestTimerPlugin::classTypeName();
    app().loadConfigFile("config.json");
    LOG_INFO << "Server starting on http://0.0.0.0:8080";
    app().run();
    return 0;
}
