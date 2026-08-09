#include <WiFi.h>

#define DEBUG_SERVER

#include "config.h"
#include "leds.h"
#include "animations.h"
#include "webserver.h"



// Web Interface HTML/CSS/JavaScript


void setup() {
    randomSeed(esp_random());
    initLeds();
    initAnimations(getLeds());
    initWebServer();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }

#ifdef DEBUG_SERVER
    Serial.begin(115200);

    Serial.println("");
    Serial.println("Connected!");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
#endif

}

void loop() {
    cleanupWebSocketClients();
    if (isAnimationRunning()) {
        updateAnimations();
        showLeds();
    }
}
