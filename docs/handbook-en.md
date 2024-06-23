# Handbook [en]

The HaniMandl manual provides an overview of the basic configuration 
of the HaniMandl firmware and its operation.


## Hardware structure

It is recommended that you only connect the servo to the pinch valve 
after you have switched on the electronics for the first time. 
The servo automatically moves to the zero position.

The rods can then be connected and the servo positions can be finely 
adjusted using the servo setup.


### Rotary control variants

Either a potentiometer or a rotary encoder is used as a rotary control.
When using a rotary encoder with a button, the button functions as a selection button.

If a potentiometer or rotary encoder without a button is used, the green button 
in the setup is used as a selection. The "tare" function in manual mode and the 
direct adjustment of the correction value and the filling quantity in automatic 
mode are only available with a rotary encoder with an integrated button.


## Setup

In switch position II (or I depending on how the 3-way switch was installed)
various basic settings can be made:

1. **Tare**

An empty weight ("tare") can be defined for each filling quantity or the 
corresponding glass. The stored tare values are displayed in the menu.

Setting: Select the filling quantity, place the empty glass and save 
using the selection button.

2. **Calibrate**

Menu-guided calibration of the load cell.

3. **Correction**

Setting the correction value. Depending on the temperature and consistency 
of the honey or the filling quantity in the filling container, the set value 
is exceeded due to the inertia of the system. The correction value is used 
to adjust this or to add a few grams more in order to meet the guidelines.

Setting: Set the value using the rotary control and confirm with selection.

4. **Fill quantity**

Selection of the filling quantity

5. **Automatic**

Setting the two automatic functions "Auto-start" and "Auto-correction" as well 
as the (margin) value for the auto-correction.

- Auto-start starts the filling process when a suitable, empty glass is placed on top.

- Auto-correction determines an automatic correction value in order to fill 
the glasses up to the filling quantity (+ margin) when the pressure in the container drops.

6. **Servo angle**

Definition of the minimum and maximum opening angle of the servo as well as 
the angle for the fine dosing.

- The minimum opening angle should close the pinch valve with minimal margin.

- The maximum opening angle limits the stroke of the servo in order to adapt 
the servo to the mechanics.

The servo remains open at least up to the fine dosing angle until the target 
weight (fill quantity + correction) is reached and then closes completely. 
Should be adjusted depending on the consistency. A larger value here leads 
to faster filling, a smaller value to more precise quantities.

Setting: In manual mode, slowly move the servo to the desired opening. 
Then set the value under "W=" here. The angles can also be set directly
from the setup via the "Live setup" option. 
Be careful when using a potentiometer or when the filling container is full.

7. **Clear Preferences**

Resets all presets (after confirmation).

The scale calibration must then be repeated and all values need to be set again.


## Operation

### Manual operation

In manual operation, the opening angle of the servo is determined directly using the rotary control.
The absolute and relative servo angles are displayed in the top line.

The servo is activated using the green button and deactivated using the red button. 
The current status is displayed using the play/pause symbol on the left.

The weight is permanently displayed. The current weight can be set as tare using 
the selection button, i.e. the scale jumps to zero and counts from the
current weight. It is reset by pressing a button when the scale is empty.

The mode is used for manual filling and for determining the values for the maximum
and minimum angle of the servo for the setup.


### Automatic operation

The automatic mode has two operating modes, which are selected using the "Auto-start" setup. 
The active auto-start is displayed in the top line with "AS". 
The absolute and relative opening angle can also be found there.

The bottom line shows the values for the correction and the filling quantity. 
An active auto-correction can be recognized there because the correction value 
is given as "ak=" instead of "k=".

The automatic mode must be activated using the green button. 
Red stops the automatic and closes the servo. 
The opening angle of the servo can be limited using the rotary control.


#### Autostart not active

If the auto-start is not active, the servo only starts when an empty or partially filled 
glass is on the scale. 
This glass is filled up to the set fill quantity, then the program stops.

A new filling must be activated again using the green button (semi-automatic).

**Attention:** Since partially filled glasses are also filled, a different weight of 
the empty glass cannot be taken into account. 
A half-full 125g glass can trigger the filling with 500g.


#### Auto-start active

The auto-start mode is also activated using the green button (play/pause symbol).

The filling process then starts automatically as soon as a suitable, empty glass is placed on top. 
When the filled glass is removed and another glass is placed on top, the next 
filling process starts without further confirmation (fully automatic).

The weight of the glass is used as tare for this filling process to compensate for 
fluctuations in the empty glasses.

A partially filled glass is not filled! Pressing the start button again forces the filling process.

If no filling process is active (i.e. the tap is closed), the correction value and 
the filling quantity for adjustment can be selected directly in both modes using 
the selection button. The value to be adjusted flashes.


### Auto-correction

The active auto-correction (display "ak=" bottom left) automatically adjusts the 
filling quantity as the pressure in the filling container drops. 
The target weight is set in the automatic setup via the margin (fill quantity + margin).

## Firmware

### Settings in the Arduino code

The behaviour of the code is controlled by several `#define` variables.

```cpp
#define HARDWARE_LEVEL 2
  1 for original pinout layout with switch on pin 19/22/21
  2 for new layout for "New Heltec Wifi Kit 32" (V2) with switch on pin 23/19/22

#define SERVO_EXTENDED
  activate if a machine is updated with software 0.1.4 and the servo does not close completely
  or the servo is generally not moved far enough.
  The new code uses default values for controlling the servo in order to address more servos. 
  The 0.1.4 code had special values where the 0 position was lower.
  The values of the 0.1.4 code can be activated using this define. 
  Alternative: adjust hardware
  
#define ROTARY_SCALE 2
  Different rotary encoders provide different increments per level. 
  This can be adjusted here.
  Examples: KY-040 = 2, HW-040 = 1
  If no rotary encoder is used, the define should be set to 1

#define USE_ROTARY
  activate if a rotary encoder is used to control the interface.

#define USE_ROTARY_SW  //  Use the rotary button
  activate if the rotary also has a button function.
  ATTENTION: if a potentiometer (see below) is used, this variable should be deactivated!
  Exception: a third button is installed. Not supported!

#define USE_POTI  //  Use the potentiometer -> ATTENTION, normally also deactivate USE_ROTARY_SW!
  activate if a potentiometer is used instead of a rotary to control the interface
  It is strongly recommended to use a rotary encoder!

#define ERROR CORRECTION_WAAGE  //  if weight jumps occur, these can be intercepted here
  Attention, can slow down the weighing process. 
  Check load cells / HX711 first!

#define QUETSCHHAHN_LINKS
  Invert servo if the pinch valve is opened from the left. 
  At least one example of such a bucket is known
```

The other defines and variables do not need to be adjusted for a standard circuit.


### Build

Just type:

```
make
```

After successfully building it, you will find firmware images at

- `.pio/build/heltec/firmware.bin`
- `.pio/build/heltec/firmware.elf`


### Notes
- The project was developed on the [Heltec WiFi Kit 32], an ESP32 with an OLED display.
  However, it should also run on other ESP32 devices.
- Please use the latest version of the HX711 library from https://github.com/bogde/HX711,
  the older version does not run on an ESP32.

### Binary files

- [hani-mandl-heltec-v2]
- [hani-mandl-heltec-v3]

The file for the Heltec V2 was compiled with the following parameters for the
board Heltec ESP32 Arduino > Wifi Kit 32:

```cpp
#define HARDWARE_LEVEL 2        //  1 = original layout with switch on pin 19/22/21
                                //  2 = layout for V2 with switch on pin 23/19/22

#define SERVO_EXTENDED          //  define if the hardware was built with the old program 
                                //  code with potentiometer or the servo does not move enough
                                //  Otherwise the servo will remain open a few degrees in the stop position! 
                                //  Check after the update!

#define ROTARY_SCALE 2          //  in which steps does our rotary encoder jump.
                                //  Examples: KY-040 = 2, HW-040 = 1, set to 1 for potentiometer operation

#define USE_ROTARY              //  Use rotary

#define USE_ROTARY_SW           //  Use rotary button
```

```{tip}
Instructions for flashing the binary file can be found here:

- [Install HaniMandl 0.2.11]
- [Install HaniMandl 0.2.13]
```


[Heltec WiFi Kit 32]: https://community.hiveeyes.org/t/heltec-wifi-kit-32-esp32-mit-kleinem-oled/1498
[hani-mandl-heltec-v2]: https://github.com/hiveeyes/hanimandl/files/14157991/hani-mandl.bin.zip
[hani-mandl-heltec-v3]: https://github.com/ClemensGruber/hani-mandl/releases/download/v0.2.13/hani-mandl_v0.2.13_heltec-v3-combined.bin
[HaniMandl 0.2.11 aufspielen]: https://hanimandl.de/2020/12/23/firmware-binary-flashen/
[HaniMandl 0.2.13 aufspielen]: https://community.hiveeyes.org/t/wie-bekomme-ich-am-einfachsten-die-hanimandl-software-aufs-board-oder-binary-datei-mit-espressif-tool-flashen/3825
