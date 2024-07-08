# Automated Greenhouse Project

This project represents software support for a demonstration model of a greenhouse for automatic regulation of microclimate parameters. 

## Installation
Before using the system, it is necessary to download the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf) and support for [ESP RainMaker](https://github.com/espressif/esp-rainmaker).

The directory **greenhouse** should be placed in *esp-rainmaker\examples\Configuration*. 

## Adjusting settings
In the ESP-IDF terminal, use the command  
```bash
idf.py menuconfig
```
to access the **Greenhouse configuration menu**. Here, you can adjust the settings:
- **Sensor configuration**
  - sampling period settings
- **Actuator configuration**
  - heater and water pump settings
- **Power save**
  - microcontroller power-saving settings

# Flashing the code
Before flashing the code, make sure the power is **disconnected** from the control unit of the demonstration model by using the rocker switch on the back of the unit. Open the unit and connected the microcontroller to the computer. Program the microcontroller using the command:
```bash
idf.py -p PORT flash monitor 
```
# Connecting to the app
A **QR code** will be printed in the ESP-IDF terminal, which needs to be scanned using the [ESP RainMaker Mobile App](https://play.google.com/store/apps/details?id=com.espressif.rainmaker&hl=en&pli=1). After scanning the code, follow further instructions from the app. After successfully connecting to the app, unplug the system from the computer, place the control unit in the greenhouse and power it on.

