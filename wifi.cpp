#include <Arduino.h>
#include <WiFi.h>

#include "wifi.h"
#include "config.h"

bool initWiFi()
{
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
