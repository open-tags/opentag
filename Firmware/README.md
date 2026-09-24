# Prototype firmware

Arduino / PlatformIO experiments for the early opentag hardware. For current opentag U1 setup, use the [current documentation](https://open-tags.com/docs/).

## Build status

**A clean checkout is missing dependencies required by the firmware.**

The active environment in [platformio.ini](platformio.ini) is `xiaoble_adafruit_nrf52`, using the `xiaoble_adafruit` board and a custom Nordic nRF52 platform. The older `esp32-c3-devkitm-1` configuration is commented out and is not an active build target.

The checkout has these known dependency gaps:

- [src/main.cpp](src/main.cpp) includes `niq.h`, but that header is not present.
- The configuration searches `lib/niq/Inc` and links `niq-m4-hfp-1.1.0.0` and `niq-m4-sfp-1.1.0.0`. Those NIQ files are not tracked; `lib/niq` is excluded by the existing [.gitignore](.gitignore).
- Several configured include paths under `src/Comm`, `src/Helpers`, `src/AppConfig`, and `src/fira`, plus `lib/HAL/Inc`, are also absent.

Recover the matching dependencies and confirm their license terms before attempting a build. The custom platform URL is not pinned to a revision, so the configuration also does not capture a reproducible historical toolchain. Resolving the missing files alone does not establish a working build.

## Build after restoring dependencies

Install [PlatformIO Core](https://docs.platformio.org/page/core.html), then run from this repository's root:

```sh
cd Firmware
pio run -e xiaoble_adafruit_nrf52
```

This command selects the environment currently defined in the project; it is not a verified successful build. Confirm the board revision and pin assignments before uploading firmware to hardware.

To remove local build output:

```sh
pio run --target clean
```

[Back to the prototype overview](../README.md)
