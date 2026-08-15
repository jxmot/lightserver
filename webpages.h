#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

// Metadata describing one embedded static web page and its HTTP route.
struct WebPage
{
    const char* path;
    WebRequestMethodComposite method;
    const char* contentType;
    const char* content;
};

// Add new page entries here in webpages.cpp when new pages are created.
// The final entry must have a null content pointer to mark the end of the list.
extern const WebPage webPages[];
