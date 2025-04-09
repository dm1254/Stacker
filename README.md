# Stacker game using Arduino Uno R3

# Required components
* Arduino Uno R3
* 2x 8x8 LED matrix(MAX7219 display driver)
* 2x push buttons
* 1x speaker
* 2x 10k ohm resistors
* 1x 150 ohm resistor

# Libraries and Tools  
## Using PlatformIO with VS Code
Install the library using PlatformIOs library manager in 3 different ways
### 1. Depend on it
Add
```
lib_deps =
    majicdesigns/MD_MAX72XX@^3.5.1
```
to your platformio.ini config file 
### 2. Include it
Include the library at the top of main project file
```
#include <MD_MAX72xx.h>
```
### 3. Command Line Interface 
1. Open PlatformIO Core CLi
2. Go to project directory where platformio.ini is located
3. Use pio pkg install command to install library
 ```
 pio pkg install --library "majicdesigns/MD_MAX72XX@^3.5.1"
 ```
## Using Arduino IDE
Install the library through Arduinos library manager
