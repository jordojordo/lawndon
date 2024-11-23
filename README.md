[![Build Tests](https://github.com/jordojordo/lawndon/actions/workflows/tests.yaml/badge.svg?event=schedule)](https://github.com/jordojordo/lawndon/actions/workflows/tests.yaml)

# Lawndon Lite

> Note: Lawndon is a WIP and the construction can be modified entirely to fit your desired needs.

> *October 2024 Update*: I have rebuilt lawndon with a new chassis, a separate ESC for the mower motor and 4x4 capabilities, wiki will be updated soon...

Before turning Lawndon into an autonomous mower, the first step is to create a remote controlled mower which can tackle any terrain. This can be accomplished by recycling any "outdated" electric mower you can find, I found a suitable [24v Worx mower](https://www.worx.com/24v-cordless-lawn-mower-wg782.html) which someone was throwing out. Eventually, I would like to construct a complete build for Lawndon that is reproducable by anyone with a 3d printer.

The main idea is to have a high torque, low power mower to be efficient and powerful enough to climb hills. In order to acheive this I used 2 [360KV brushless motors](https://www.rctimer.com/rctimer-5010-360kv-multicopter-brushless-motor-p0233.html) to power a 56:1 gearbox that is attached to tracks. The gears and tracks are based on [Rctestflight's Snowcat design](https://www.thingiverse.com/thing:4308626).

## Install

> Note: You will need to have the [`arduino-cli` installed](https://arduino.github.io/arduino-cli/0.19/installation/).

Currently the only build that is supported is for the [Arduino Mega 2560](https://store.arduino.cc/products/arduino-mega-2560-rev3) or it's [FQBN](https://arduino.github.io/arduino-cli/0.19/commands/arduino-cli_board_listall/): `arduino:avr:mega`.

You download and extract the [latest release](https://github.com/jordojordo/lawndon-lite/releases), then install the binaries:

```sh
wget https://github.com/jordojordo/lawndon-lite/releases/download/0.1.0/arduino.avr.mega.tar.gz

mkdir build
tar -xcvf arduino.avr.mega.tar.gz -C build

arduino-cli upload --input-dir build
```

> Note: For more detailed instructions on uploading to a board follow the [arduino-cli documentation](https://arduino.github.io/arduino-cli/0.19/commands/arduino-cli_upload/).

## Build

To build, first determine your board's [FQBN](https://arduino.github.io/arduino-cli/0.19/commands/arduino-cli_board_listall/). Then clone the directory, install the libraries, and compile:

```sh
arduino-cli lib install IBusBM Servo
arduino-cli compile -b <FQBN> -e --libraries "~/Arduino/libraries/IBusBM,~/Arduino/libraries/Servo" ./lawndon/
```

## Configuration
 
Refer to the [Wiki](https://github.com/jordojordo/lawndon-lite/wiki) for prerequisites, required materials, and configuration instructions.

## Videos

I'll continually update the [Videos section](https://github.com/jordojordo/lawndon-lite/wiki/Videos) of the Wiki to document the current progress of Lawndon.

## TODO

- Design printable chassis
- Configure RC reciever to be agnostic
- Create proper wire routing scheme