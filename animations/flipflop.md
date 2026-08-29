# FlipFlop Animation Specification

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
  * When LEDs turn off(change states), or change to a different color there should a "fade" between the even and odd LEDs. For example, if even are blue and odd are red then when they switch the even will fade to red and the odd will fade to blue. The duration of the fade should be 20% of the current time between state changes.
  * The animation continues indefinitely until another animation is selected or the LEDs are turned off.
