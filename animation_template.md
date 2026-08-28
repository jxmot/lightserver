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

## Notes

### Brightness behavior

For example, an animation might:

* use global brightness normally (i.e. global brightness applies to all LEDs)
* use brightness independently for different LED groups
* have its own brightness modulation
* have fixed brightness regardless of the global setting

### Special behavior

This would cover things like:

* behavior when starting
* behavior when stopping
* whether LEDs should begin in a particular state
* randomization
* synchronization requirements
* behavior at the end of a cycle
* anything unusual about the animation

### Speed/timing

* Speed/timing:
  * minimum speed:
  * maximum speed:
  * speed behavior:

## Animation Template

Use the following format when describing an animation

* Name:
* What the LEDs should do:

* Colors:
  * primary color:
    * Can the primary color be changed by the client?:
  * secondary color:
    * Can the secondary color be changed by the client?:

* Brightness behavior:

* Speed/timing:
  * minimum speed:
  * maximum speed:
  * speed behavior:

* Special behavior:

## Animation Examples

Here is an example of how an animation is described. When you create a new animation place it into its own markdown file and start the file with "# AnimationName". Name the file with the animation name. Using 'Example 1' below:

* file name: flipflop.md
* first line: # FlipFlop

### Example 1

* Name: FlipFlop

* What the LEDs should do:
  Divide the LEDs into odd-numbered and even-numbered LEDs. Odd-numbered LEDs turn on while even-numbered LEDs turn will turn off or turn on with an optional secondary color. Then the states reverse.

* Colors:
  * Primary color: The configured primary animation color.
    * Can the primary color be changed by the client?: Yes
  * Secondary color: yellow
    * Can the secondary color be changed by the client?: Yes

* Brightness behavior:
  Global brightness applies to the LEDs that are turned on.

* Speed/timing:
  * Minimum speed: 1000 ms between state changes.
  * Maximum speed: 100 ms between state changes.
  * Speed behavior: The speed control determines the time between each odd/even state change. Each state remains active for the selected duration before the LEDs switch to the opposite state.

* Special behavior:
  * When LEDs turn off(change states), or change to a different color there should a "fade" between the even and odd LEDs. For example, if even are blue and odd are red then when they switch the even will fade to red and the odd will fade to blue. The duration of the fade should be 20% of the current time between state changes.
  * The animation continues indefinitely until another animation is selected or the LEDs are turned off.
