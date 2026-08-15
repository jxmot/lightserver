#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void initWebSocket(AsyncWebServer& server);
void cleanupWebSocketClients();
