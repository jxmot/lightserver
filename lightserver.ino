
#define DEBUG_SERVER

#include "config.h"
#include "wifi.h"
#include "leds.h"
#include "animations.h"
#include "webserver.h"
#include "websocket.h"

void setup() {
    randomSeed(esp_random());

#ifdef DEBUG_SERVER
    Serial.begin(115200);
#endif

    initLeds();
    initAnimations(getLeds());

    initWiFi();

#ifdef DEBUG_SERVER
    Serial.println("");
    Serial.println("Connected!");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
#endif

    initWebServer();

#ifdef DEBUG_SERVER
    Serial.println("HTTP server started");
#endif

    startAnimation("ready");
}

void loop() {
    cleanupWebSocketClients();
    if (isAnimationRunning()) {
        updateAnimations();
        showLeds();
    }
}
