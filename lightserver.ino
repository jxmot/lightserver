#include "config.h"
#include "wifi.h"
#include "leds.h"
#include "animations.h"
#include "webserver.h"
#include "websocket.h"

void setup() {
#ifdef DEBUG_SERVER
    Serial.begin(115200);
#endif

    initLeds();
    initAnimations(getLeds());

    bool wifiConnected = initWiFi();

#ifdef DEBUG_SERVER
    Serial.println("");

    if (wifiConnected)
    {
        Serial.println("Connected!");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC address: ");
        Serial.println(WiFi.macAddress());
    }
    else
    {
        Serial.println("ERROR: WiFi connection failed.");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(password);
        Serial.print("Connection timeout (ms): ");
        Serial.println(WiFiConnectionTimeout);
    }
#endif

    if (wifiConnected)
    {
        initWebServer();

#ifdef DEBUG_SERVER
        Serial.println("HTTP server started");
#endif

        startAnimation("ready");
    }
    else
    {
        startAnimation("wifierror");
    }
}

void loop() {
    cleanupWebSocketClients();

    if (consumeWiFiDisconnectedEvent())
        startAnimation("wifierror");

    if (isAnimationRunning()) {
        updateAnimations();
        showLeds();
    }
}
