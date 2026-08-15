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

The project is organized so that the major areas of responsibility are separated into modules. The main program (`lightserver.ino`) is intentionally small and primarily responsible for initialization and the main loop.

The project also uses a small, explicit public API between modules. Implementation details such as animation types, animation-name lookup tables, WebSocket broadcast functions, and LED storage remain private to their respective `.cpp` files.

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
           └───────────┘       └─────┬──────┘       └─────┬─────┘
                                     │                    │
                                     │                    │
                                     ▼                    │
                              ┌─────────────┐             │
                              │   WebPages  │             │
                              │ webpages.*  │             │
                              └─────────────┘             │
                                     │                    │
                                     ▼                    │
                              ┌─────────────┐             │
                              │  WebSocket  │─────────────┤
                              │ websocket.* │             │
                              └──────┬──────┘             │
                                     │                    │
                                     ▼                    │
                              ┌─────────────┐             │
                              │  Commands   │             │
                              │ commands.*  │             │
                              └──────┬──────┘             │
                                     │                    │
                         ┌───────────┴───────────┐        │
                         ▼                       ▼        │
                  ┌──────────────┐        ┌─────────────┐ │
                  │  Animations  │        │     LEDs    │◄┘
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
4. Connects to Wi-Fi.
5. Starts the HTTP server.
6. Reports that the HTTP server has started.
7. Starts the five-second `Ready` animation.

`loop()`:

1. Cleans up inactive WebSocket clients.
2. Updates the animation engine when an animation is running.
3. Sends the current LED data to the physical strip.

The detailed initialization of each subsystem is deliberately kept out of `lightserver.ino`.

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
```

`PixelCount` and `PixelPin` are defined in `config.cpp` and declared `extern` in `config.h`.

Keeping the definitions in `config.cpp` prevents every source file that includes `config.h` from creating its own definition.

---

### `wifi.h` / `wifi.cpp`

Responsible for connecting the ESP32 to the configured Wi-Fi network.

Public API:

```cpp
void initWiFi();
```

The rest of the Wi-Fi implementation is private to the module.

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

void setManualLed(
    uint16_t index,
    bool state,
    const RgbColor* color = nullptr,
    const uint8_t* brightness = nullptr);

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
| `ready` | Green flashing startup animation |
| `chase` | Theater-chase style animation |
| `scan` | Scanning LED animation |
| `fade` | Color fade animation |
| `rainbow` | Rainbow-cycle animation |
| `fire` | Fire-effect animation |
| `twinkle` | Starry/twinkle animation |
| `heart` | Heartbeat animation |
| `off` | No animation |

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

The HTTP server is responsible for registering the embedded web pages and initializing the WebSocket subsystem.

The actual page definitions are deliberately kept out of this module.

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

The page list is terminated by an entry whose `content` pointer is `nullptr`.

This design makes adding another page straightforward:

1. Create the HTML page.
2. Add its embedded content to `webpages.cpp`.
3. Add a `WebPage` entry containing its route, HTTP method, content type, and content.

The web-server implementation can then register the pages by iterating through the list rather than requiring a separate `server.on()` call for every page.

This keeps page content and page routing information together.

---

### `pixelstrip.h`

Provides the project's LED-strip abstraction.

The LED and animation modules use this abstraction rather than exposing the underlying LED implementation throughout the application.

---

## Web Interface

The current embedded pages include:

```text
/        → index.html
/led     → led.html
```

The HTML files are kept separately from `webserver.cpp`, making it easier to add additional pages without increasing the size or complexity of the web-server implementation.

The browser communicates with the ESP32 through normal HTTP requests for pages and WebSockets for real-time LED control and state updates.

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
    ├── initWiFi()
    │
    ├── initWebServer()
    │
    ├── "HTTP server started"
    │
    └── startAnimation("ready")
             │
             ▼
       5-second green
       flashing animation
```

After startup, `loop()` continuously updates the animation engine when necessary and services WebSocket client cleanup.

---

## Refactoring Design Principles

The current architecture follows several principles used throughout the refactoring:

### Keep public APIs small

Headers expose only functions and types required by other modules. Implementation details remain in `.cpp` files.

### Separate transport from application behavior

The WebSocket layer does not implement LED or animation commands. It passes commands to `commands.cpp`.

### Keep state ownership local

LED state belongs to `leds.cpp`.

Animation state belongs to `animations.cpp`.

### Prefer consolidated state snapshots

When another module needs several related values, a snapshot structure is preferred over several individual getter calls.

Examples:

```cpp
AnimationStateSnapshot
ManualLedState
```

### Keep page definitions separate from the server

HTML content and route metadata belong to `webpages.cpp`; HTTP server setup belongs to `webserver.cpp`.

### Preserve behavior during refactoring

Refactoring changes are intended to improve organization and maintainability without changing the externally observable behavior of the LED controller.

---

## Adding a New Animation

A new animation generally requires changes only to `animations.cpp`.

The implementation should:

1. Add a new private `AnimationType`.
2. Add the name/type pair to `animationNames[]`.
3. Add the rendering behavior to `animationCallback()`.

The public API does not need to change merely because another animation is added.

---

## Adding a New Web Page

To add a page:

1. Create the HTML content.
2. Add the page content to `webpages.cpp`.
3. Add a `WebPage` entry to the `webPages[]` array.
4. Ensure the final array entry remains the end-of-list marker.

The HTTP server does not need another hard-coded `server.on()` call for the new page.

---

## Build Environment

The project is intended for the ESP32 using the Arduino IDE and the libraries already used by the project, including:

- ESP32 Arduino core
- `ESPAsyncWebServer`
- `ArduinoJson`
- `NeoPixelBus` / `NeoPixelAnimator` functionality used by the project

The project has been developed and tested incrementally by compiling and running each refactoring step on the target ESP32 hardware.

---

## Current Status

The project has completed the planned refactoring sequence through **Step 48**.

The current codebase has been tested after each incremental change, including:

- Compilation
- Startup sequence
- `Ready` animation
- Built-in animations
- Manual LED control
- LED colors
- WebSocket communication
- Browser state updates
- HTTP page loading

The Step 48 version is the current refactored baseline.
