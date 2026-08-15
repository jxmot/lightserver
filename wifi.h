#pragma once

// Connect to the configured WiFi network.
// Returns true if connected within WiFiConnectionTimeout, otherwise false.
bool initWiFi();
bool consumeWiFiDisconnectedEvent();
