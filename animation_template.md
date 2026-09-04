# New Lightserver Animation Specification

This document is the template for specifying a new animation for the Lightserver project. The specification describes the intended LED behavior and is used as the basis for creating a standalone `.h` / `.cpp` animation implementation.

Usage: Provide the Lightserver source ZIP and a completed animation specification to the AI. The resulting animation should normally be implemented as its own source files in the `animations/` folder.

## Current Behaviors

* By default, animations remember their brightness and animation speed settings while Lightserver is running. Returning to an animation loads its remembered settings.
* If an animation has client-configurable colors, it remembers those colors when it is turned off. Returning to the animation loads its remembered colors.
* Animation speed is represented as a duration in milliseconds between animation state changes. Each animation can define its own minimum and maximum duration.
* The Show Controller receives each animation's speed range from the ESP32 and configures the Animation Speed slider for the selected animation.

## Conventions

To be consistent across animations, use the following when describing an animation:

* LEDs are numbered 1 through N, where N is the quantity of LEDs as defined in `config.cpp`. You can refer to them as LED1, LED2, etc. LED references beyond the defined quantity are ignored. LEDs can also be described as "odd LEDs" or "even LEDs".
* Colors can be described using color names or hexadecimal RGB values such as `#FF0000`.
* Time can be described in seconds or milliseconds. Duty cycles and percentages can also be used when appropriate.
* If something does not apply, use N/A.
* Some items can be answered with "yes" or "no". If left blank, "no" is implied.
* **Minimum Speed** means the slowest end of the animation's timing range. State the actual duration between state changes.
* **Maximum Speed** means the fastest end of the animation's timing range. State the actual duration between state changes.

## Notes

### Brightness Behavior

Describe how the animation uses brightness. For example, an animation might:

* use global brightness normally, meaning global brightness applies to all LEDs
* use brightness independently for different LED groups
* have its own brightness modulation
* use fixed brightness regardless of the global setting

### Special Behavior

Describe behavior such as:

* behavior when starting
* behavior when stopping
* whether LEDs should begin in a particular state
* randomization
* synchronization requirements
* behavior at the end of a cycle
* fading or transitions
* anything unusual about the animation

### Speed/timing

Describe the timing between state changes at the slowest and fastest speeds. The values should be actual durations in milliseconds or seconds. If the animation has a repeating cycle, also describe the relationship between the state-change duration and the complete cycle duration.

* Speed/timing:
  * Minimum Speed:
  * Maximum Speed:
  * Speed Behavior:

## Animation Specification Template

Use the following format when specifying the behavior of an animation:

* Name:
* What the LEDs should do:

* Colors:
  * Primary Color:
    * Can the primary color be changed by the client?:
  * Secondary Color:
    * Can the secondary color be changed by the client?:

* Brightness Behavior:

* Speed/timing:
  * Minimum Speed:
  * Maximum Speed:
  * Speed Behavior:

* Special Behavior:

## Implementation Notes

When the specification is implemented:

* Put the animation source in its own `animations/<name>.h` and `animations/<name>.cpp` files.
* If the animation source needs `PixelStrip`, include it from the project root with:

  `#include "../pixelstrip.h"`

* Keep animation-specific implementation details in the animation's own source files whenever practical.
* Add the animation to the animation manager's private type and registration information.
* Report the animation's minimum and maximum duration to the manager so the web client can configure its speed slider correctly.
* Increment `ProtocolVersion::ShowInfo` when adding or changing an animation's reported information.
* Do not modify documentation as part of the animation implementation unless specifically requested.

## Animation Example

When creating a new animation, place its specification into its own markdown file and start the file with `# AnimationName`. Name the file using the animation name, for example `animations/flipflop.md`.

### Example: FlipFlop

* Name: FlipFlop

* What the LEDs should do:
  Divide the LEDs into odd-numbered and even-numbered LEDs. Odd-numbered LEDs turn on with the primary color. Even-numbered LEDs either turn off or turn on with the secondary color, depending on whether the secondary color is enabled. Then the states reverse.

* Colors:
  * Primary Color: The configured primary animation color.
    * Can the primary color be changed by the client?: Yes
  * Secondary Color: yellow
    * Can the secondary color be changed by the client?: Yes

* Brightness Behavior:
  Global brightness applies to the LEDs that are turned on.

* Speed/timing:
  * Minimum Speed: 1000 ms between state changes.
  * Maximum Speed: 100 ms between state changes.
  * Speed Behavior: The speed control determines the time between each odd/even state change. Each state remains active for the selected duration before the LEDs switch to the opposite state.

* Special Behavior:
  * When LEDs turn off (change states), or change to a different color, there should be a "fade" between the even and odd LEDs. For example, if even LEDs are blue and odd LEDs are red, when they switch, the even LEDs fade to red and the odd LEDs fade to blue. The duration of the fade should be 20% of the current time between state changes.
  * The animation continues indefinitely until another animation is selected or the LEDs are turned off.
