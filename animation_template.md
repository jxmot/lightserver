# New Lightserver Animation Description

The following descibes an animation that will be used by the Lightserver project.

## Current Behaviors

* By default animations will remember their set brightness and animation speed settings. Returning to an animation will load its remembered settings. 
* If an animation has client-configurable colors then it will remember them when it is turned off. Returning to an animation will load its remembered colors.

## Conventions

To be consistent across animations, use the following when describing an animation:

* LEDs are numbered 1 through N, where N is the quantity of LEDs as defined in `config.cpp`. And you can refer to them as LED1, LED2, etc. LED references beyond the defined quantity are ignored. Or the leds can be referred to as "odd leds" or "even leds".
* Colors can be decribed using color names, or hexidecmal RGB (such as #FF0000)
* Time can be described in seconds, milliseconds, or by duty cycle or percentages
* If something does not apply then use N/A
* Some items can be answered with "yes" or "no", if left blank then "no" is implied
  * Do not change items that are pre-filled below

## Animation Template

* Name: 
* What the LEDs should do: 
* Colors:
  * primary color: 
    * Can the primary color be changed by the client?:
  * secondary color:
    * Can the secondary color be changed by the client?:
* Speed/timing:
* Whether brightness and speed controls should affect it: YES
* Whether it should remember its settings: YES

## Animation Examples

### Example 1

* Name: FlipFlop
* What the LEDs should do: the LEDs should alternate, between even and odd numbered leds. When the even leds are on, the odd leds are off and vice versa. There should be a "cross fade" between even and odd leds. don't just switch between them. the cross fade duration should os relative based on the speed set by the client. 
* Colors:
  * primary color: green
    * Can the primary color be changed by the client?: yes
  * secondary color: yellow
    * Can the secondary color be changed by the client?: yes
* Speed/timing: the lowest speed should be 1 second each for even and odd leds, or a duty cycle of 2 seconds. That includes the cross fade time. Cross fading should use 25% of the odd or even time. For example, the even and odd LEDS will be on (cross-fading) at the same time for 25% of their half of the dury cycle.
* Whether brightness and speed controls should affect it: YES
* Whether it should remember its settings: YES




