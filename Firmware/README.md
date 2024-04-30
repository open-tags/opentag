How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-espressif32/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to Firmware
$ cd opentag/Firmware/

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e esp32-c3-devkitm-1

# Upload firmware for the specific environment
$ pio run -e esp32-c3-devkitm-1 --target upload

# Clean build files
$ pio run --target clean
```