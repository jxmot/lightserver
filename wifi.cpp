#include <Arduino.h>
#include <WiFi.h>

#include "wifi.h"
#include "config.h"

void initWiFi()
{
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
}
