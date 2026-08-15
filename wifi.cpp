#include <Arduino.h>
#include <WiFi.h>

#include "wifi.h"
#include "config.h"

namespace
{
    volatile bool wifiDisconnectedEvent = false;

    void onWiFiEvent(WiFiEvent_t event)
    {
        if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
            wifiDisconnectedEvent = true;
    }
}

bool initWiFi()
{
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.begin(ssid, password);

    const unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime >= WiFiConnectionTimeout)
            return false;

        delay(500);
    }

    return true;
}

bool consumeWiFiDisconnectedEvent()
{
    if (!wifiDisconnectedEvent)
        return false;

#ifdef DEBUG_SERVER
    Serial.println("Lost connection to WiFi access point");
#endif

    wifiDisconnectedEvent = false;
    return true;
}
