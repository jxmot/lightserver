#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

void initWebServer();
void cleanupWebSocketClients();
