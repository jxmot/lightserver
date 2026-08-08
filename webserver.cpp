#include "webserver.h"

// The HTML resources and WebSocket callback remain in lightserver.ino
// during this intermediate refactoring step.
extern const char index_html[] PROGMEM;
extern const char ledctl_html[] PROGMEM;

extern void onEvent(
    AsyncWebSocket *server,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len
);

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

void initWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", index_html);
    });

    server.on("/leds", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", ledctl_html);
    });

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.begin();

#ifdef DEBUG_SERVER
    Serial.println("HTTP server started");
#endif
}

AsyncWebSocket& getWebSocket()
{
    return ws;
}
