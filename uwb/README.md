# UWB Trilateration System

This project is designed to create a trilateration system using Ultra-Wideband (UWB) technology. The system consists of three anchors and one tag, which work together to determine the position of the tag relative to the anchors. This README provides detailed instructions on setting up and using the system.

## Directory Structure

- **setup/**: Contains files necessary for calibrating the UWB modules.
- **anchor/**: Contains the code for setting up the anchor modules.
- **tag/**: Contains the code for setting up the tag module.

## Setup Instructions

### Calibrating UWB Modules

To calibrate the UWB modules, navigate to the `setup` directory. This directory contains scripts and configuration files necessary for the calibration process. Follow these steps:

1. **Connect the UWB Modules**: Ensure that all UWB modules are properly connected to your computer.
2. **Measure Distance**: Before starting the calibration, place the tag and an anchor a certain distance apart. Then measure the distance between the tag and the anchor. Update the `targetDistance` variable in the `anchor.ino` file with this measured value. The positions of the tag and anchor will be used during your calibration.
3. **Upload Calibration Code**: 
   - Upload the `setup/tag/tag.ino` file to the tag module.
   - Upload the `setup/anchor/anchor.ino` file to each of the anchor modules.
4. **Run Calibration Script**: Place the tag and one anchor at the previous measured positions. When turned on the calibration process begins and will print out a final delay value.
5. **Verify Calibration**: After running the calibration, write down the final delay value for the specific anchor.
6. **Repeat**: Repeat the process for each anchor.

## Trilateration System Setup

### Setting Up Anchors

The `anchor` directory contains the necessary files to set up the anchor modules. Follow these steps:

1. **Modify Anchor Configuration**: For each anchor, modify the `shortAddress` and `macAddress` in the `anchor.ino` file to match the specific short address and MAC address for that anchor used in the previous calibration step. Also, ensure the `antennaDelay` is set to the value determined during the calibration step.
2. **Upload Code to Anchors**: Use the `anchor.ino` file to upload the necessary code to each of the three anchor modules. This can be done using the Arduino IDE or any compatible platform.
3. **Position Anchors**: Place the anchors in a known configuration within the environment where the trilateration will occur. Ensure that the positions are accurately measured and recorded.

### Setting Up the Tag

The `tag` directory contains the files needed to set up the tag module. Follow these steps:

1. **Upload Code to Tag**: Use the `tag.ino` file to upload the necessary code to the tag module.
2. **Activate the Tag**: Once the code is uploaded, activate the tag and ensure it is communicating with the anchors.

## Running the System

Once all modules are set up and calibrated, you can begin using the trilateration system. The tag will communicate with the anchors to determine its position based on the time it takes for signals to travel between the tag and each anchor.

## Troubleshooting

- **Communication Issues**: Ensure that all modules are powered and within range of each other.
- **Calibration Errors**: Re-run the calibration setup and verify connections.
