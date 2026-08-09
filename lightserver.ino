#include <WiFi.h>

#define DEBUG_SERVER

#include "config.h"
#include "leds.h"
#include "animations.h"
#include "webserver.h"



// Web Interface HTML/CSS/JavaScript


void setup() {
    randomSeed(esp_random());

#ifdef DEBUG_SERVER
    Serial.begin(115200);
#endif

    initLeds();
    initAnimations(getLeds());

    // The WebServer must not be started until the ESP32 TCP/IP stack
    // has been initialized by a successful WiFi connection.
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

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

    // WebServer is now running. Start the five-second Ready animation.
    startAnimation("ready");
}

void loop() {
    cleanupWebSocketClients();
    if (isAnimationRunning()) {
        updateAnimations();
        showLeds();
    }
}
