#include "webserver.h"
#include "webpages.h"
#include "websocket.h"

static AsyncWebServer server(80);

void initWebServer()
{
    for (size_t i = 0; webPages[i].content != nullptr; ++i)
    {
        const WebPage& page = webPages[i];
        server.on(
            page.path,
            page.method,
            [page](AsyncWebServerRequest *request)
            {
                request->send(200, page.contentType, page.content);
            });
    }

    // The server checks handlers in the order they are added.
    const WebPage& errPage = errPages[static_cast<size_t>(ErrorPageTypes::Page404)];
    server.onNotFound([errPage](AsyncWebServerRequest *request) {
        request->send(404, errPage.contentType, errPage.content);
    });

    initWebSocket(server);

    server.begin();
}

