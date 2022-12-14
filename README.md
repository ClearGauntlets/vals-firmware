# OpenGloves Firmware
This is a firmware implementation meant to work with the [Steam VR Driver OpenGloves](https://github.com/LucidVR/opengloves-driver). It is meant to support many different types of hardware and can be configured to work with specific implementations.

# Firmware
Follow the instruction from LucidGloves, except use the firmware files in this repository.

Open the firmware files located in the open-gloves folder.
By default the firmware is configured for an arduino ESP32 using serial.
To configure the firmware, change the defines in the Config.h file.

**Configuration instructions are here: [Configuration Instructions](https://github.com/LucidVR/lucidgloves/wiki/Firmware-Setup-and-Customization-Tutorial/).**

## Using Arduino-CLI

The Arduino-CLI is a little more ergonomic to use than the IDE.

First, download and install [Arduino-CLI](https://github.com/arduino/arduino-cli)


Read the [getting started guide](https://arduino.github.io/arduino-cli/0.27/getting-started/)

Or just do this:

```
arduino-cli config init
arduino-cli core update-index
```

Locate your `arduino-cli.yaml` config file (check the output of `arduino-cli config init`) and add the following line under `board _manager -> additional_urls`
```
    - 'https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json'
```

```
arduino-cli core install esp32:esp32
arduino-cli compile --fqbn esp32:esp32:esp32
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32 .
```

## Tested boards:
* ESP-WROOM-32

## Supported Communication Methods:
* USB Serial
* Bluetooth Serial (On ESP32 boards)

# SteamVR Compatibility (OpenGloves)
This project uses the OpenGloves OpenVR driver for compatibility with SteamVR, which is downloadable on Steam:
https://store.steampowered.com/app/1574050/OpenGloves/

# Credits
### Upstream repository available on GitHub:
https://github.com/LucidVR/opengloves-driver

### Special Thanks:
* [LucasVRTech](https://github.com/lucas-vrtech) of the LucidGlove project
* [danwillim](https://github.com/danwillm) of OpenGloves project
* [TwidgeVR](https://github.com/TwidgeVR) guides on how to make homemade flex sensors
* [Vals](https://github.com/Valsvirtuals) of the ProtoGlove Project
* [Crylog](https://github.com/ciborg971) Multiplexer support + RGB LEDS support
* [AikoKiss](https://github.com/AikoKiss) Beta testing
* [MinyStreem](https://github.com/MinyStreem) Wifi Support
