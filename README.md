# Lightserver

## Overview

**Lightserver** is an ESP32-based LED controller that provides a web interface for controlling an addressable LED strip.

The application combines:

- An ESP32 microcontroller
- An addressable LED strip driven through the `PixelStrip` abstraction
- Wi-Fi connectivity
- An asynchronous HTTP web server
- A WebSocket connection for real-time control and state updates
- A collection of built-in LED animations
- Individual/manual LED control
- Embedded HTML pages stored separately from the web-server implementation
- Wi-Fi connection and disconnect monitoring

The project is organized so that the major areas of responsibility are separated into modules. The main program (`lightserver.ino`) is intentionally small and primarily responsible for initialization and the main loop.

The project also uses a small, explicit public API between modules. Implementation details such as animation types, animation-name lookup tables, WebSocket broadcast functions, Wi-Fi event handling, and LED storage remain private to their respective `.cpp` files.

---

## Architecture

At a high level, the application is organized like this:

```text
                           ┌──────────────────────┐
                           │    lightserver.ino   │
                           │  setup() / loop()    │
                           └──────────┬───────────┘
                                      │
                 ┌────────────────────┼────────────────────┐
                 │                    │                    │
                 ▼                    ▼                    ▼
           ┌───────────┐       ┌────────────┐       ┌───────────┐
           │   Wi-Fi   │       │ Web Server │       │    LEDs   │
           │  wifi.*   │       │ webserver.*│       │  leds.*   │
           └─────┬─────┘       └─────┬──────┘       └─────┬─────┘
                 │                   │                    │
                 │                   ▼                    │
                 │            ┌─────────────┐             │
                 │            │   WebPages  │             │
                 │            │ webpages.*  │             │
                 │            └──────┬──────┘             │
                 │                   │                    │
                 │                   ▼                    │
                 │            ┌─────────────┐             │
                 │            │  WebSocket  │─────────────┤
                 │            │ websocket.* │             │
                 │            └──────┬──────┘             │
                 │                   │                    │
                 │                   ▼                    │
                 │            ┌─────────────┐             │
                 │            │  Commands   │             │
                 │            │ commands.*  │             │
                 │            └──────┬──────┘             │
                 │                   │                    │
                 │       ┌───────────┴───────────┐        │
                 │       ▼                       ▼        │
                 │ ┌──────────────┐        ┌─────────────┐│
                 └►│  Animations  │        │     LEDs    │◄┘
                   │ animations.* │        │   leds.*    │
                   └──────┬───────┘        └─────────────┘
                          │
                          ▼
                   ┌──────────────┐
                   │ PixelStrip   │
                   │ pixelstrip.h │
                   └──────────────┘
```

### Responsibilities

#### `lightserver.ino`

The application entry point.

`setup()`:

1. Initializes the serial interface when debugging is enabled.
2. Initializes the LED subsystem.
3. Initializes the animation subsystem.
4. Attempts to connect to Wi-Fi.
5. Reports the Wi-Fi connection result when debugging is enabled.
6. If Wi-Fi succeeds, initializes the HTTP/WebSocket server.
7. If Wi-Fi succeeds, starts the five-second `Ready` animation.
8. If Wi-Fi fails, skips web-server initialization and starts the `wifierror` animation.

`loop()`:

1. Cleans up inactive WebSocket clients.
2. Checks for a Wi-Fi disconnected event.
3. Starts the `wifierror` animation when a Wi-Fi disconnect event is consumed.
4. Updates the animation engine when an animation is running.
5. Sends the current LED data to the physical strip.

The detailed initialization of each subsystem is deliberately kept out of `lightserver.ino`.

---

## Wi-Fi Behavior

Wi-Fi configuration is stored in `config.cpp`.

`WiFiConnectionTimeout` specifies how long `initWiFi()` waits for the initial connection. The current value is **90 seconds**:

```cpp
const uint32_t WiFiConnectionTimeout = 90000;
```

`initWiFi()` returns:

- `true` when a Wi-Fi connection is established before the timeout.
- `false` when the timeout expires without a connection.

When the initial Wi-Fi connection fails:

- An error is reported on the serial connection when `DEBUG_SERVER` is enabled.
- The configured SSID is reported.
- The configured password is reported.
- The configured connection timeout is reported.
- The HTTP server and WebSocket server are **not** initialized.
- The `wifierror` animation is started.

The application does not currently attempt to reconnect automatically after an initial connection failure.

### Wi-Fi disconnect events

The Wi-Fi module registers a station-disconnected event using the current Arduino-ESP32 event API:

```cpp
WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
```

The event callback records the disconnect rather than directly starting an animation. `loop()` calls `consumeWiFiDisconnectedEvent()` and starts `wifierror` from the normal application context.

Automatic Wi-Fi reconnection after a disconnect is not currently implemented.

---

## Module API

### `config.h` / `config.cpp`

Contains application configuration.

Public configuration values include:

```cpp
extern const uint16_t PixelCount;
extern const uint8_t PixelPin;

extern const char *ssid;
extern const char *password;

extern const uint32_t WiFiConnectionTimeout;
```

`PixelCount`, `PixelPin`, and `WiFiConnectionTimeout` are defined in `config.cpp` and declared `extern` in `config.h`.

Keeping the definitions in `config.cpp` prevents every source file that includes `config.h` from creating its own definition.

---

### `wifi.h` / `wifi.cpp`

Responsible for the initial Wi-Fi connection and Wi-Fi disconnect event handling.

Public API:

```cpp
bool initWiFi();
bool consumeWiFiDisconnectedEvent();
```

`initWiFi()` waits up to `WiFiConnectionTimeout` milliseconds for the initial connection.

`consumeWiFiDisconnectedEvent()` returns `true` once when a station-disconnected event has been recorded, then clears the event. It returns `false` when no unconsumed disconnect event exists.

The Wi-Fi event callback itself remains private to `wifi.cpp`.

---

### `leds.h` / `leds.cpp`

Owns the physical LED strip and manual LED state.

Public API:

```cpp
void initLeds();
PixelStrip& getLeds();
void showLeds();
void clearLeds();

void setManualMode(bool enabled);
bool isManualMode();

void setManualLed(uint16_t index, bool state, const RgbColor* color = nullptr, const uint8_t* brightness = nullptr);

struct ManualLedState
{
    bool on;
    RgbColor color;
    uint8_t brightness;
};

bool getManualLedState(uint16_t index, ManualLedState& state);
void clearManualLeds();
void showManualLeds();
```

The module keeps its LED storage private.

`ManualLedState` provides a consolidated way to read an LED's complete manual state instead of requiring separate calls for its on/off state, color, and brightness.

`showManualLeds()` renders the current manual state to the physical LED strip.

---

### `animations.h` / `animations.cpp`

Owns the animation engine and all animation-specific implementation details.

Public API:

```cpp
void initAnimations(PixelStrip& strip);
void startAnimation(const String& name);
void stopAnimation();
bool isAnimationRunning();
void updateAnimations();

void setAnimationColor(const RgbColor& color);
void setAnimationBrightness(uint8_t brightness);
void setAnimationDuration(uint16_t duration);

String getAnimationName();
```

Current animation state can be retrieved through:

```cpp
struct AnimationStateSnapshot
{
    String name;
    RgbColor color;
    uint8_t brightness;
    uint16_t duration;
};

void getAnimationState(AnimationStateSnapshot& snapshot);
```

The individual animation state getters were intentionally removed in favor of this consolidated snapshot API.

Implementation-only details such as `AnimationType`, the animation-name table, the `NeoPixelAnimator` object, animation state storage, and animation callbacks remain private to `animations.cpp`.

#### Built-in animations

The current animation names are:

| Name | Description |
|---|---|
| `wifierror` | Fixed red Wi-Fi error indication: 75% brightness for 250 ms, then off for 500 ms, repeating indefinitely |
| `ready` | Green flashing startup animation |
| `chase` | Theater-chase style animation |
| `scan` | Scanning LED animation |
| `fade` | Color fade animation |
| `rainbow` | Rainbow-cycle animation |
| `fire` | Fire-effect animation |
| `twinkle` | Starry/twinkle animation |
| `heart` | Heartbeat animation |
| `off` | No animation |

The `wifierror` animation is a system-status animation. Its color, brightness, and timing are independent of the normal user-controlled animation settings.

The `Ready` animation is intentionally different from the normal animation timing. It runs for **5 seconds** and flashes the LEDs green with a half-second on/half-second off cycle.

---

### `commands.h` / `commands.cpp`

Contains application-level commands.

This module separates command processing from the WebSocket transport layer.

Public API:

```cpp
CommandResult processCommand(JsonDocument& doc);
```

Commands currently supported include:

- `pattern`
- `brightness`
- `speed`
- `color`
- `led`
- `alloff`

The command module changes application state but does not directly manage WebSocket transmission.

It returns a `CommandResult` containing the required state-broadcast order:

```cpp
enum class CommandBroadcastOrder : uint8_t
{
    None,
    AnimationOnly,
    ManualOnly,
    ManualThenAnimation,
    AnimationThenManual
};
```

This keeps WebSocket-specific broadcasting out of the application command handlers.

---

### `websocket.h` / `websocket.cpp`

Provides the WebSocket transport used by the web interface.

Public API:

```cpp
void initWebSocket(AsyncWebServer& server);
void cleanupWebSocketClients();
```

The module is responsible for:

1. Receiving WebSocket messages.
2. Parsing JSON.
3. Passing commands to `processCommand()`.
4. Broadcasting resulting state changes.
5. Sending current state when a WebSocket client connects.
6. Cleaning up inactive clients.

The actual command behavior is implemented in `commands.cpp`.

WebSocket state messages are generated by private functions such as:

```text
broadcastAnimationState()
broadcastManualState()
```

Both use the private `broadcastJson()` helper for the common JSON serialization/transmission operation.

---

### `webserver.h` / `webserver.cpp`

Owns the asynchronous HTTP server.

Public API:

```cpp
void initWebServer();
```

The HTTP server:

1. Registers the normal web pages from `webPages[]`.
2. Registers the configured error-page handler.
3. Initializes the WebSocket subsystem.
4. Starts the HTTP server.

The actual page definitions are deliberately kept out of this module.

The web server is initialized only after a successful initial Wi-Fi connection.

---

### `webpages.h` / `webpages.cpp`

Contains the embedded web pages and their route metadata.

Each page is described by a `WebPage` structure:

```cpp
struct WebPage
{
    const char* path;
    WebRequestMethodComposite method;
    const char* contentType;
    const char* content;
};
```

Normal pages are stored in:

```cpp
extern const WebPage webPages[];
```

Error pages are stored in:

```cpp
extern const WebPage errPages[];
```

Error page types are identified by:

```cpp
enum class ErrorPageTypes : uint8_t
{
    Page404
};
```

Both page arrays use a final entry whose `content` pointer is `nullptr` as the end-of-list marker.

The current normal routes are:

```text
/       → index.html
/leds   → led.html
```

The current error route is a custom HTTP 404 response using `404.html`.

In `webserver.cpp`, each normal page is copied into the registered request lambda with:

```cpp
[page]
```

This is intentional. Each route retains its own page metadata after the page-registration loop has completed.

---

### `pixelstrip.h`

Provides the project's LED-strip abstraction.

The LED and animation modules use this abstraction rather than exposing the underlying LED implementation throughout the application.

---

## Web Interface

The current web pages are:

```text
/        → Show Controller
/leds    → LED Controller
```

The Show Controller provides:

- Animation selection
- Global brightness
- Animation speed
- Feature color
- WebSocket connection status

The LED Controller provides:

- Individual LED selection
- Individual LED color
- Individual LED brightness
- All Off
- WebSocket connection status

The browser communicates with the ESP32 through normal HTTP requests for pages and WebSockets for real-time LED control and state updates.

A request for an unknown HTTP path receives the embedded 404 page.

---

## WebSocket / Command Flow

A typical browser command follows this path:

```text
Browser
   │
   │ JSON over WebSocket
   ▼
websocket.cpp
   │
   │ processCommand()
   ▼
commands.cpp
   │
   ├──────────────► animations.cpp
   │
   └──────────────► leds.cpp
   │
   ▼
CommandResult
   │
   ▼
websocket.cpp
   │
   ├── broadcastAnimationState()
   └── broadcastManualState()
```

This separation is intentional:

- **WebSocket** handles transport.
- **Commands** handle application commands.
- **Animations** handle animation behavior.
- **LEDs** handle physical LED state and rendering.

---

## Startup Sequence

The normal startup sequence is:

```text
ESP32 reset
    │
    ▼
setup()
    │
    ├── Serial initialization
    │
    ├── initLeds()
    │
    ├── initAnimations()
    │
    └── initWiFi()
             │
       ┌─────┴─────┐
       │           │
   SUCCESS       FAILURE
       │           │
       ▼           ▼
initWebServer()  report error
       │         start wifierror
       │
       ▼
"HTTP server started"
       │
       ▼
startAnimation("ready")
```

If the initial Wi-Fi connection fails, the web server and WebSocket server are not initialized.

After a successful connection, if Wi-Fi subsequently disconnects:

```text
Wi-Fi disconnect event
          │
          ▼
   wifi.cpp records
       the event
          │
          ▼
     loop() calls
consumeWiFiDisconnectedEvent()
          │
          ▼
startAnimation("wifierror")
```

Automatic Wi-Fi reconnection is not currently implemented.

---

## Adding a New Animation

A new normal animation generally requires changes only to `animations.cpp`.

The implementation should:

1. Add a new private `AnimationType`.
2. Add the name/type pair to `animationNames[]`.
3. Add the rendering behavior to `animationCallback()`.

The public API does not need to change merely because another animation is added.

System-status animations such as `wifierror` may use fixed parameters instead of the normal user-controlled animation state.

---

## Adding a New Web Page

To add a normal page:

1. Create the HTML content.
2. Add the page content to `webpages.cpp`.
3. Add a `WebPage` entry to the `webPages[]` array.
4. Ensure the final entry remains the end-of-list marker.

To add an error page:

1. Add the embedded HTML content to `webpages.cpp`.
2. Add an entry to `errPages[]`.
3. Add an appropriate value to `ErrorPageTypes` when indexed access is required.
4. Ensure the final entry remains the end-of-list marker.

The HTTP server does not need another hard-coded `server.on()` call for each new page.

---

## Build Environment

The project is intended for the ESP32 using the Arduino IDE and the libraries already used by the project, including:

- ESP32 Arduino core
- `ESPAsyncWebServer`
- `ArduinoJson`
- `NeoPixelBus` / `NeoPixelAnimator` functionality used by the project

The project has been developed and tested incrementally on the target ESP32 hardware.

---

## Current Status

The current baseline includes:

- Modular LED, animation, Wi-Fi, command, web-server, web-page, and WebSocket code
- A 90-second configurable initial Wi-Fi connection timeout
- Wi-Fi connection failure reporting
- `wifierror` animation on initial Wi-Fi connection failure
- Wi-Fi station-disconnect event detection
- `wifierror` animation when an established Wi-Fi connection is lost
- HTTP/WebSocket server startup only after successful initial Wi-Fi connection
- A dedicated embedded 404 page
- Consolidated animation and manual LED state APIs
- Embedded web pages managed through page metadata tables

Automatic Wi-Fi reconnection is intentionally not implemented at this time.
