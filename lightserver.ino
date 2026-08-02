#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <ArduinoJson.h>

#define DEBUG_SERVER

//#include "config.h"
extern const char *ssid;
extern const char *password;


// NeoPixel Configuration
const uint16_t PixelCount = 4; // Change to the number of LEDs you have
const uint8_t PixelPin = 16;    // ESP32 GPIO pin for data

// Using NeoEsp32Rmt0Ws2812xMethod for precise hardware timing on ESP32
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip(PixelCount, PixelPin);
NeoPixelAnimator animations(1); // 1 active animation coordinator

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Global States
enum Pattern {
    OFF,
    THEATER_CHASE,
    SCAN,
    COLOR_FADE,
    RAINBOW_CYCLE,
    FIRE_EFFECT,
    STARRY_TWINKLE,
    HEARTBEAT
};
Pattern currentPattern = OFF;

uint16_t animDuration = 2000; 
uint8_t globalBrightness = 128; // 0-255 scale
RgbColor userColor(255, 0, 0);   // Default color chosen by user

// Manual LED control state
bool manualMode = false;
bool manualLedState[PixelCount] = { false };

RgbColor manualColor(255, 0, 0);
uint8_t manualBrightness = 128;

// Fire Effect state array
uint8_t heat[PixelCount];

// Helper to apply brightness scale smoothly to raw colors
RgbColor ApplyBrightness(RgbColor baseColor) {
    return RgbColor(
        (baseColor.R * globalBrightness) / 255,
        (baseColor.G * globalBrightness) / 255,
        (baseColor.B * globalBrightness) / 255
    );
}

// Web Interface HTML/CSS/JavaScript
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Show Controller</title>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background: #1e1e1e; color: #fff; margin: 0; padding: 20px; }
        .container { max-width: 500px; margin: auto; background: #2b2b2b; padding: 20px; border-radius: 10px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); }
        button { padding: 12px 20px; font-size: 16px; margin: 6px; border: none; border-radius: 5px; cursor: pointer; color: white; width: 45%; }
        .btn-off { background: #d9534f; width: 93%; }
        .btn-anim { background: #337ab7; }
        .btn-active { background: #5cb85c !important; font-weight: bold; }
        .control-group { margin: 20px 0; text-align: left; background: #3a3a3a; padding: 15px; border-radius: 6px; }
        label { display: block; font-weight: bold; margin-bottom: 5px; }
        input[type=range] { width: 100%; margin-bottom: 10px; }
        input[type=color] { width: 100%; height: 40px; border: none; border-radius: 4px; cursor: pointer; }
        .disabled { opacity: 0.3; pointer-events: none; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Show Controller</h1>
        <div id="status" style="color: #ffcc00; margin-bottom: 10px;">Connecting to WebSockets...</div>
        
        <button class="btn-off" id="btn-off" onclick="sendPattern('off')">Turn Off</button><br>
        <button class="btn-anim" onclick="window.location.href='/leds'">LEDs</button><br>
        <button class="btn-anim" id="btn-chase" onclick="sendPattern('chase')">Chase</button>
        <button class="btn-anim" id="btn-scan" onclick="sendPattern('scan')">Scan</button>
        <button class="btn-anim" id="btn-fade" onclick="sendPattern('fade')">Color Fade</button>
        <button class="btn-anim" id="btn-rainbow" onclick="sendPattern('rainbow')">Rainbow</button>
        <button class="btn-anim" id="btn-fire" onclick="sendPattern('fire')">Fire Effect</button>
        <button class="btn-anim" id="btn-twinkle" onclick="sendPattern('twinkle')">Twinkle</button>
        <button class="btn-anim" id="btn-heart" onclick="sendPattern('heart')">Heartbeat</button>

        <div class="control-group">
            <label for="brightness">Global Brightness</label>
            <input type="range" id="brightness" min="10" max="255" value="128" oninput="sendSlider('brightness', this.value)">
            
            <label for="speed">Animation Speed</label>
            <input type="range" id="speed" min="200" max="8000" value="2000" oninput="sendSlider('speed', this.value)">
        </div>

        <div class="control-group" id="colorPickerContainer">
            <label for="colorPicker">Feature Color</label>
            <input type="color" id="colorPicker" value="#ff0000" onchange="sendColor(this.value)">
        </div>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        
        window.addEventListener('load', initWebSocket);

        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onopen = function() { document.getElementById('status').innerText = 'Connected'; };
            websocket.onclose = function() { document.getElementById('status').innerText = 'Disconnected. Retrying...'; setTimeout(initWebSocket, 2000); };
            websocket.onmessage = onMessage;
        }

function onMessage(event) {
    var data = JSON.parse(event.data);

    // Ignore manual LED messages
    if(data.type && data.type !== "animation")
        return;

    // Highlight active animation button
    document.querySelectorAll('button').forEach(b => b.classList.remove('btn-active'));

    if(data.pattern !== 'off') {
        let button = document.getElementById('btn-' + data.pattern);
        if(button)
            button.classList.add('btn-active');
    } else {
        document.getElementById('btn-off').classList.add('btn-active');
    }

    // Update animation color
    if(data.color !== undefined) {
        document.getElementById("colorPicker").value = data.color;
    }

    // Update animation brightness
    if(data.brightness !== undefined) {
        document.getElementById("brightness").value = data.brightness;
    }

    // Update animation speed
    if(data.speed !== undefined) {
        document.getElementById("speed").value = 8200 - data.speed;
    }

    // Enable color picker only for color-based animations
    var cp = document.getElementById('colorPickerContainer');
    if(['chase', 'fade', 'fire', 'scan'].includes(data.pattern)) {
        cp.classList.remove('disabled');
    } else {
        cp.classList.add('disabled');
    }
}
/*
        function onMessage(event) {
            var data = JSON.parse(event.data);
            
            // Highlight active button
            document.querySelectorAll('button').forEach(b => b.classList.remove('btn-active'));
            if(data.pattern !== 'off') {
                document.getElementById('btn-' + data.pattern).classList.add('btn-active');
            } else {
                document.getElementById('btn-off').classList.add('btn-active');
            }
            if (data.color) {
                document.getElementById("colorPicker").value = data.color;
            }
            if(data.brightness) {
                document.getElementById("brightness").value = data.brightness;
            }
            if(data.speed) {
                document.getElementById("speed").value = 8200 - data.speed;
            }
            // Context-aware color picker visibility control
            var cp = document.getElementById('colorPickerContainer');
            if(['chase', 'fade', 'fire', 'scan'].includes(data.pattern)) {
                cp.classList.remove('disabled');
            } else {
                cp.classList.add('disabled');
            }
        }
*/

        function sendPattern(p) { websocket.send(JSON.stringify({type: 'pattern', value: p})); }
        
        // Invert slider so lower ms value means "faster" representation for user comfort
        function sendSlider(type, val) { 
            let targetVal = val;
            if(type === 'speed') targetVal = (8200 - val); 
            websocket.send(JSON.stringify({type: type, value: parseInt(targetVal)})); 
        }
        function sendColor(val) { 
            websocket.send(JSON.stringify({type: 'color', value: val}));
        }
    </script>
</body>
</html>
)rawliteral";

const char led_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>LED Controller</title>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background: #1e1e1e;
            color: #fff;
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 500px;
            margin: auto;
            background: #2b2b2b;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 10px rgba(0,0,0,0.3);
        }
        button {
            padding: 12px 20px;
            font-size: 16px;
            margin: 6px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            color: white;
            width: 45%;
        }
        .btn-off {
            background: #d9534f;
            width: 93%;
        }
        .btn-led {
            background: #337ab7;
        }
        .btn-active {
            background: #5cb85c !important;
            font-weight: bold;
        }
        .control-group {
            margin: 20px 0;
            text-align: left;
            background: #3a3a3a;
            padding: 15px;
            border-radius: 6px;
        }
        label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
        }
        input[type=range] {
            width: 100%;
            margin-bottom: 10px;
        }
        input[type=color] {
            width: 100%;
            height: 40px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>LED Controller</h1>
    <div id="status" style="color:#ffcc00; margin-bottom:10px;">
        Connecting to WebSockets...
    </div>
    <button class="btn-led" onclick="window.location.href='/'">
        Shows
    </button>
    <button class="btn-off" onclick="allOff()">
        All Off
    </button>
    <div id="ledButtons">
    </div>
    <div class="control-group">
        <label for="brightness">
            LED Brightness
        </label>
        <input 
            type="range"
            id="brightness"
            min="10"
            max="255"
            value="128">
        <label for="colorPicker">
            LED Color
        </label>
        <input
            type="color"
            id="colorPicker"
            value="#ff0000">
    </div>
</div>
<script>
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var pixelCount = 0;
var ledStates = [];

window.addEventListener('load', initWebSocket);

function initWebSocket()
{
    websocket = new WebSocket(gateway);
    websocket.onopen = function()
    {
        document.getElementById('status').innerText = "Connected";
    };

    websocket.onclose = function()
    {
        document.getElementById('status').innerText = "Disconnected. Retrying...";
        setTimeout(initWebSocket,2000);
    };
    websocket.onmessage = onMessage;
}

function createLedButtons(count)
{
    pixelCount = count;
    var container = document.getElementById("ledButtons");
    container.innerHTML="";
    ledStates = new Array(count).fill(false);

    for(let i=0;i<count;i++)
    {
        let button=document.createElement("button");
        button.className="btn-led";
        button.id="led-"+i;
        button.innerText="LED "+(i+1);
        button.onclick=function()
        {
            toggleLed(i);
        };
        container.appendChild(button);
    }
}

function toggleLed(index)
{
    var state = !ledStates[index];
    ledStates[index]=state;
    var message =
    {
        type:"led",
        index:index,
        state:state
    };

    if(state)
    {
        message.color = document.getElementById("colorPicker").value;
        message.brightness = parseInt(document.getElementById("brightness").value);
    }
    websocket.send(JSON.stringify(message));
}

function allOff()
{
    websocket.send(JSON.stringify(
    {
        type:"alloff"
    }));
}

function updateButtons()
{
    for(let i=0;i<ledStates.length;i++)
    {
        let button = document.getElementById("led-"+i);

        if(!button) continue;

        if(ledStates[i])
            button.classList.add("btn-active");
        else
            button.classList.remove("btn-active");
    }
}

function onMessage(event)
{
    var data = JSON.parse(event.data);
    if(data.type !== "manual")
        return;

    if(data.leds)
    {
        ledStates=data.leds;
        if(pixelCount !== data.leds.length)
        {
            createLedButtons(data.leds.length);
            ledStates=data.leds;
        }
        updateButtons();
    }

    if(data.color)
        document.getElementById("colorPicker").value=data.color;

    if(data.brightness !== undefined)
        document.getElementById("brightness").value=data.brightness;
}
</script>
</body>
</html>
)rawliteral";

String getCurrentPatternString();
void broadcastAnimationState();
void broadcastManualState();
void stopAnimationMode();
void stopManualMode();
void showManualLeds();

String getCurrentPatternString()
{
    switch(currentPattern)
    {
        case THEATER_CHASE: return "chase";
        case SCAN:          return "scan";
        case COLOR_FADE:    return "fade";
        case RAINBOW_CYCLE: return "rainbow";
        case FIRE_EFFECT:   return "fire";
        case STARRY_TWINKLE:return "twinkle";
        case HEARTBEAT:     return "heart";
        default:            return "off";
    }
}

void stopAnimationMode()
{
    if (animations.IsAnimating())
        animations.StopAnimation(0);

    currentPattern = OFF;

    strip.ClearTo(RgbColor(0,0,0));
    strip.Show();
    
    broadcastAnimationState();
}
}

void stopManualMode()
{
    manualMode = false;

    for(uint16_t i=0;i<PixelCount;i++)
        manualLedState[i]=false;
    
    broadcastManualState();
}

void showManualLeds()
{
    strip.ClearTo(RgbColor(0,0,0));

    for(uint16_t i=0;i<PixelCount;i++)
    {
        if(manualLedState[i])
        {
            strip.SetPixelColor(
                i,
                RgbColor(
                    manualColor.R * manualBrightness / 255,
                    manualColor.G * manualBrightness / 255,
                    manualColor.B * manualBrightness / 255));
        }
    }

    strip.Show();
}

void broadcastAnimationState()
{
    StaticJsonDocument<256> doc;

    doc["type"] = "animation";
    doc["pattern"] = getCurrentPatternString();

    char colorString[8];
    sprintf(colorString,
            "#%02X%02X%02X",
            userColor.R,
            userColor.G,
            userColor.B);

    doc["color"] = colorString;
    doc["brightness"] = globalBrightness;
    doc["speed"] = animDuration;

    String output;
    serializeJson(doc, output);

    ws.textAll(output);
}

void broadcastManualState()
{
    StaticJsonDocument<512> doc;

    doc["type"] = "manual";

    char colorString[8];
    sprintf(colorString,
            "#%02X%02X%02X",
            manualColor.R,
            manualColor.G,
            manualColor.B);

    doc["color"] = colorString;
    doc["brightness"] = manualBrightness;

    JsonArray leds = doc.createNestedArray("leds");

    for(uint16_t i=0;i<PixelCount;i++)
        leds.add(manualLedState[i]);

    String output;
    serializeJson(doc, output);

    ws.textAll(output);
}

// Animation Logic Frame Callback
void AnimationLoopCallback(const AnimationParam& param) {
    if (currentPattern == OFF) return;

    if (currentPattern == COLOR_FADE) {
        RgbColor targetColor;
        if (param.progress < 0.5f) {
            targetColor = RgbColor::LinearBlend(RgbColor(0,0,0), userColor, param.progress / 0.5f);
        } else {
            targetColor = RgbColor::LinearBlend(userColor, RgbColor(0,0,0), (param.progress - 0.5f) / 0.5f);
        }
        strip.ClearTo(ApplyBrightness(targetColor));
    } 
    
    else if (currentPattern == THEATER_CHASE) {
        uint16_t pos = (uint16_t)(param.progress * PixelCount);
        if (pos >= PixelCount) pos = PixelCount - 1;
        strip.ClearTo(RgbColor(0,0,0));
        strip.SetPixelColor(pos, ApplyBrightness(userColor));
    } 
    
    else if (currentPattern == SCAN) {
        float t = param.progress * 2.0f;
        if (t > 1.0f)
            t = 2.0f - t;
    
        uint16_t pos = round(t * (PixelCount - 1));
        strip.ClearTo(RgbColor(0, 0, 0));
        strip.SetPixelColor(pos, ApplyBrightness(userColor));
    }
    
    else if (currentPattern == RAINBOW_CYCLE) {
        for (uint16_t i = 0; i < PixelCount; i++) {
            float hue = param.progress + ((float)i / PixelCount);
            if (hue > 1.0f) hue -= 1.0f;
            strip.SetPixelColor(i, ApplyBrightness(HslColor(hue, 1.0f, 0.5f)));
        }
    }

    else if (currentPattern == FIRE_EFFECT) {
        uint8_t steps = _max(1, (uint8_t)(20.0f * param.progress));
    
        static uint8_t lastStep = 0;
        if (steps != lastStep) {
            lastStep = steps;
    
            for (int i = 0; i < PixelCount; i++) {
                heat[i] = (heat[i] * 4) / 5;
            }
    
            if (random(100) < 20) {
                int idx = random(PixelCount);
                heat[idx] = _min(255, heat[idx] + random(160,255));
            }
        }
    
        for (uint16_t i = 0; i < PixelCount; i++) {
            float ratio = heat[i] / 255.0f;
    
            RgbColor fireColor =
                RgbColor::LinearBlend(RgbColor(0,0,0), userColor, ratio);
    
            if (ratio > 0.5f) {
                fireColor =
                    RgbColor::LinearBlend(
                        fireColor,
                        RgbColor(255,255,100),
                        (ratio - 0.5f) * 2.0f);
            }
    
            strip.SetPixelColor(i, ApplyBrightness(fireColor));
        }
    }

    else if (currentPattern == STARRY_TWINKLE) {
        uint8_t steps = _max(1, (uint8_t)(20.0f * param.progress));
    
        static uint8_t lastTwinkleStep = 0;
    
        if (steps != lastTwinkleStep) {
            lastTwinkleStep = steps;
    
            for (uint16_t i = 0; i < PixelCount; i++) {
                RgbColor c = strip.GetPixelColor(i);
                strip.SetPixelColor(i,
                    RgbColor(c.R * 0.85f,
                            c.G * 0.85f,
                            c.B * 0.85f));
            }
    
            if (random(100) < 30) {
                uint16_t star = random(PixelCount);
                strip.SetPixelColor(
                    star,
                    ApplyBrightness(
                        RgbColor(random(200,255),
                                random(200,255),
                                255)));
            }
        }
    }

    else if (currentPattern == HEARTBEAT) {
        float intensity = 0.0f;
        float progress = param.progress;
        
        if (progress < 0.15f) { 
            intensity = sin((progress / 0.15f) * PI);
        } else if (progress >= 0.25f && progress < 0.40f) { 
            intensity = sin(((progress - 0.25f) / 0.15f) * PI) * 0.7f; 
        } 
        
        RgbColor redBeat(intensity * 255, 0, 0); 
        strip.ClearTo(ApplyBrightness(redBeat));
    }

    if (param.state == AnimationState_Completed && currentPattern != OFF) {
        animations.RestartAnimation(param.index);
    }
}

/*
// Global state broadcaster utility
void broadcastState(String currentPatternString) {
    StaticJsonDocument<200> doc;
    doc["pattern"] = currentPatternString;

    char colorString[8];
    sprintf(colorString, "#%02X%02X%02X",
            userColor.R,
            userColor.G,
            userColor.B);
    doc["color"] = colorString;
    doc["brightness"] = globalBrightness;
    doc["speed"] = animDuration;

    String output;
    serializeJson(doc, output);
    ws.textAll(output);
}
*/
// Handle socket data traffic
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error) return;

        String type = doc["type"];
        
        if (type == "pattern") {
            stopManualMode();
            String val = doc["value"];
            if (val == "off") {
                // stopAnimationMode();
                currentPattern = OFF;
                animations.StopAnimation(0);
                strip.ClearTo(RgbColor(0,0,0));
                strip.Show();
            } else {
                if (val == "chase") currentPattern = THEATER_CHASE;
                else if (val == "scan") currentPattern = SCAN;
                else if (val == "fade") currentPattern = COLOR_FADE;
                else if (val == "rainbow") currentPattern = RAINBOW_CYCLE;
                else if (val == "fire") currentPattern = FIRE_EFFECT;
                else if (val == "twinkle") currentPattern = STARRY_TWINKLE;
                else if (val == "heart") currentPattern = HEARTBEAT;
                
                animations.StartAnimation(0, animDuration, AnimationLoopCallback);
            }
            broadcastAnimationState();
            //broadcastState(val);
        } 
        
        else if (type == "brightness") {
            globalBrightness = doc["value"].as<int>();
            broadcastAnimationState();
            //broadcastState(getCurrentPatternString());
        } 

        else if (type == "speed")
        {
            animDuration = doc["value"].as<int>();
            if (animations.IsAnimating())
            {
                animations.StartAnimation(
                    0,
                    animDuration,
                    AnimationLoopCallback);
            }
            broadcastAnimationState();
        }
/*
        else if (type == "speed") {
            animDuration = doc["value"].as<int>();
            if (animations.IsAnimating()) {
                animations.StartAnimation(0, animDuration, AnimationLoopCallback);
            }
            broadcastState(getCurrentPatternString());
        } 
*/
        else if (type == "color") {
            String hex = doc["value"].as<String>();
            if (hex.charAt(0) == '#') {
                hex = hex.substring(1);
            }
            long number = strtol(hex.c_str(), NULL, 16);
            userColor = RgbColor((number >> 16) & 0xFF, (number >> 8) & 0xFF, number & 0xFF);
            // Notify every connected client
            broadcastAnimationState();
            //broadcastState(getCurrentPatternString());
        }

        else if(type == "led")
        {
            uint16_t index = doc["index"];
            if(index >= PixelCount)
                return;
            bool state = doc["state"];
            // Turning on a manual LED switches modes
            if(state)
            {
                if(!manualMode)
                {
                    stopAnimationMode();
                    manualMode = true;
                }
                manualColor = RgbColor(
                    strtol(
                        doc["color"].as<String>().substring(1,3).c_str(),
                        NULL,
                        16),
                    strtol(
                        doc["color"].as<String>().substring(3,5).c_str(),
                        NULL,
                        16),
                    strtol(
                        doc["color"].as<String>().substring(5,7).c_str(),
                        NULL,
                        16)
                );
                manualBrightness = doc["brightness"].as<uint8_t>();
            }
            manualLedState[index] = state;
            showManualLeds();
            broadcastManualState();
        }
        
        else if(type == "alloff")
        {
            stopAnimationMode();
            manualMode = false;
            for(uint16_t i=0;i<PixelCount;i++)
            {
                manualLedState[i]=false;
            }
            showManualLeds();
            broadcastManualState();
        }

    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            broadcastAnimationState();
            broadcastManualState();
/*
            if(currentPattern == OFF) broadcastState("off");
            else if(currentPattern == THEATER_CHASE) broadcastState("chase");
            else if(currentPattern == SCAN) broadcastState("scan");
            else if(currentPattern == COLOR_FADE) broadcastState("fade");
            else if(currentPattern == RAINBOW_CYCLE) broadcastState("rainbow");
            else if(currentPattern == FIRE_EFFECT) broadcastState("fire");
            else if(currentPattern == STARRY_TWINKLE) broadcastState("twinkle");
            else if(currentPattern == HEARTBEAT) broadcastState("heart");
*/
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    
    randomSeed(esp_random());
    memset(heat, 0, sizeof(heat));
    
    strip.Begin();
    strip.Show();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }

#ifdef DEBUG_SERVER
    Serial.println("");
    Serial.println("Connected!");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
#endif

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });

    server.begin();
#ifdef DEBUG_SERVER
    Serial.println("HTTP server started");
#endif
}

void loop() {
    ws.cleanupClients();
    if (animations.IsAnimating()) {
        animations.UpdateAnimations();
        strip.Show();
    }
}
