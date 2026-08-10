#pragma once

#include <NeoPixelBus.h>
#include "config.h"

using PixelStrip = NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>;
